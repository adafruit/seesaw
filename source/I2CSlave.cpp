/*******************************************************************************
 * Copyright (C) Dean Miller
 * All rights reserved.
 *
 * This program is open source software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include "qpcpp.h"
#include "qp_extras.h"

#include "hsm_id.h"
#include "I2CSlave.h"
#include "event.h"
#include "bsp_gpio.h"

#include "bsp_sercom.h"
#include "SeesawConfig.h"
#include "PinMap.h"
#include "bsp_nvmctrl.h"

Q_DEFINE_THIS_FILE

using namespace FW;

static Fifo *m_inFifo;
static Fifo *m_outFifo;

static Fifo *m_defaultOutFifo;

static volatile bool slave_busy;

static volatile uint8_t bytes_received, high_byte, low_byte;

#if CONFIG_I2C_SLAVE_FLOW_CONTROL
#if CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN >= 32
#define FLOW_CONTROL_LOW            PORT->Group[PORTB].OUTCLR.reg = (1ul<<(CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN-32));
#else
#define FLOW_CONTROL_LOW            PORT->Group[PORTA].OUTCLR.reg = (1ul<<CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN);
#endif
#else
#define FLOW_CONTROL_LOW
#endif

#if CONFIG_I2C_SLAVE_FLOW_CONTROL
#if CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN >= 32
#define FLOW_CONTROL_HIGH           PORT->Group[PORTB].OUTSET.reg = (1ul<<(CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN-32));
#else
#define FLOW_CONTROL_HIGH            PORT->Group[PORTA].OUTSET.reg = (1ul<<CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN);
#endif
#else
#define FLOW_CONTROL_HIGH
#endif

#if CONFIG_I2C_SLAVE_FLOW_CONTROL
#if CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN >= 32
#define FLOW_CONTROL_INIT            gpio_init(PORTB, CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN-32, 1); //set as output
#else
#define FLOW_CONTROL_INIT            gpio_init(PORTA, CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN, 1); //set as output
#endif
#else
#define FLOW_CONTROL_INIT
#endif

I2CSlave::I2CSlave( Sercom *sercom) :
    QActive((QStateHandler)&I2CSlave::InitialPseudoState), 
    m_id(I2C_SLAVE), m_name("I2C Slave"), m_sercom(sercom),
	m_timeout(this, I2C_SLAVE_TIMEOUT) {}

QState I2CSlave::InitialPseudoState(I2CSlave * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(I2C_SLAVE_START_REQ);
    me->subscribe(I2C_SLAVE_STOP_REQ);
      
	me->subscribe(I2C_SLAVE_REQUEST);
	me->subscribe(I2C_SLAVE_RECEIVE);
	me->subscribe(I2C_SLAVE_STOP_CONDITION);
	me->subscribe(I2C_SLAVE_TIMEOUT);
	
	me->subscribe(DELEGATE_DATA_READY);
	  
    return Q_TRAN(&I2CSlave::Root);
}

QState I2CSlave::Root(I2CSlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&I2CSlave::Stopped);
            break;
        }
		case I2C_SLAVE_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&I2CSlave::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState I2CSlave::Stopped(I2CSlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case I2C_SLAVE_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new I2CSlaveStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case I2C_SLAVE_START_REQ: {
            LOG_EVENT(e);
			
#if CONFIG_EEPROM
			//see if there is an I2C addr set
			uint8_t addr = eeprom_read_byte(SEESAW_EEPROM_I2C_ADDR);
#else
			uint8_t addr = CONFIG_I2C_SLAVE_ADDR;
#endif
			uint32_t val = 0;
			addr = (addr > 0x7F ? CONFIG_I2C_SLAVE_ADDR : addr);

#if CONFIG_ADDR
			
			uint32_t mask = (1ul << PIN_ADDR_0) | (1ul << PIN_ADDR_1) | (CONFIG_ADDR_2 << PIN_ADDR_2) |
			(CONFIG_ADDR_3 << PIN_ADDR_3) | (CONFIG_ADDR_4 << PIN_ADDR_4);
			gpio_dirclr_bulk(PORTA, mask);
			gpio_pullenset_bulk(mask);
			gpio_outset_bulk(PORTA, mask);
			
			//wait for everything to settle
			for(uint32_t i=0; i<50000ul; i++){
				asm("nop");
			}
			
#if CONFIG_ADDR_2 || CONFIG_ADDR_3 || CONFIG_ADDR_4
			uint32_t addrs = gpio_read_bulk();
			val = ((addrs & (1ul << PIN_ADDR_0)) > 0) 
				| (((addrs & (1ul << PIN_ADDR_1)) > 0) << 1)
				| (((addrs & (CONFIG_ADDR_2 << PIN_ADDR_2)) > 0) << 2)
				| (((addrs & (CONFIG_ADDR_3 << PIN_ADDR_3)) > 0) << 3)
				| (((addrs & (CONFIG_ADDR_4 << PIN_ADDR_4)) > 0) << 4);
			
			val ^= 0x1F;
#else
			val = (gpio_read_bulk() >> PIN_ADDR_0);
			val ^= 0x03;
#endif

#endif

			FLOW_CONTROL_INIT

#if CONFIG_ADDR_2 || CONFIG_ADDR_3 || CONFIG_ADDR_4
			initSlaveWIRE( me->m_sercom, addr + (val & 0x1F) );
#else
			initSlaveWIRE( me->m_sercom, addr + (val & 0x03) );
#endif

			enableWIRE( me->m_sercom );
			NVIC_ClearPendingIRQ( CONFIG_I2C_SLAVE_IRQn );
			
			slave_busy = false;
			
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SDA, CONFIG_I2C_SLAVE_MUX);
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SCL, CONFIG_I2C_SLAVE_MUX);
			
			I2CSlaveStartReq const &req = static_cast<I2CSlaveStartReq const &>(*e);
			m_inFifo = req.getInFifo();
			m_defaultOutFifo = req.getOutFifo();
			m_outFifo = m_defaultOutFifo;
			bytes_received = 0;
			
			m_inFifo->Reset();
			m_outFifo->Reset();
			
#if CONFIG_ACTIVITY_LED
#if PIN_ACTIVITY_LED >= 32
            gpio_init(PORTB, PIN_ACTIVITY_LED-32, 1); //set as output
#else
            gpio_init(PORTA, PIN_ACTIVITY_LED, 1); //set as output
#endif
#endif
			
			Evt *evt = new I2CSlaveStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&I2CSlave::Started);
            break;
        }
        default: {
            status = Q_SUPER(&I2CSlave::Root);
            break;
        }
    }
    return status;
}

QState I2CSlave::Started(I2CSlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
            status = Q_TRAN(&I2CSlave::Idle);
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
			enableWIRE( me->m_sercom );
            status = Q_HANDLED();
            break;
        }
		case I2C_SLAVE_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new I2CSlaveStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(I2CSlave::Stopped);
			break;
		}
		case DELEGATE_DATA_READY:{
			DelegateDataReady const &req = static_cast<DelegateDataReady const &>(*e);
			
			//a timeout must have occured, toss out any data
			if(req.getRequesterId() == me->m_id){
				if(req.getFifo() != NULL){
					Fifo * f = req.getFifo();
					f->Reset();
				}
				else m_outFifo->Reset();
				status = Q_HANDLED();
			}
			else
			    status = Q_UNHANDLED();
			break;
		}
        default: {
            status = Q_SUPER(&I2CSlave::Root);
            break;
        }
    }
    return status;
}

QState I2CSlave::Idle(I2CSlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
			slave_busy = false;
			FLOW_CONTROL_HIGH
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
#if CONFIG_ACTIVITY_LED
#if PIN_ACTIVITY_LED >= 32
            PORT->Group[PORTB].OUTSET.reg = (1ul<<(PIN_ACTIVITY_LED-32)); //activity led on
#else
            PORT->Group[PORTA].OUTSET.reg = (1ul<<PIN_ACTIVITY_LED); //activity led on
#endif
#endif
            status = Q_HANDLED();
            break;
        }
		case I2C_SLAVE_RECEIVE: {
			LOG_EVENT(e);
			
			I2CSlaveReceive const &req = static_cast<I2CSlaveReceive const &>(*e);
			
			if((req.getLowByte() > 0 || req.getHighByte() > 0) && req.getLen() > 0){
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), req.getLen(), m_inFifo);
				QF::PUBLISH(evt, me);
				FLOW_CONTROL_HIGH
				status = Q_HANDLED();
			}
			else if(req.getLowByte() > 0 || req.getHighByte() > 0) {
				m_defaultOutFifo->Reset();	
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), 0, m_defaultOutFifo);
				QF::PUBLISH(evt, me);
				status = Q_TRAN(&I2CSlave::Busy);
			}
			else{
			    FLOW_CONTROL_HIGH
			    status = Q_HANDLED();
			}
			
			break;
		}
        default: {
            status = Q_SUPER(&I2CSlave::Started);
            break;
        }
    }
    return status;
}

QState I2CSlave::Busy(I2CSlave * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			me->m_timeout.armX(100, 100);
			
			slave_busy = true;
			
			//assume default output fifo
			m_outFifo = m_defaultOutFifo;
			
			status = Q_HANDLED();
			
			break;
		}
		case DELEGATE_DATA_READY: {
		    LOG_EVENT(e);
			DelegateDataReady const &req = static_cast<DelegateDataReady const &>(*e);
			if(req.getRequesterId() == me->m_id){
				if(req.getFifo() != NULL){
					 m_outFifo = req.getFifo();
				}
				status = Q_TRAN(&I2CSlave::Idle);
			}
			else
			    status = Q_UNHANDLED();
			break;
		}
		case I2C_SLAVE_TIMEOUT: {
			status = Q_TRAN(&I2CSlave::Idle);
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
#if CONFIG_ACTIVITY_LED
#if PIN_ACTIVITY_LED >= 32
            PORT->Group[PORTB].OUTCLR.reg = (1ul<<(PIN_ACTIVITY_LED-32)); //activity led off
#else
            PORT->Group[PORTA].OUTCLR.reg = (1ul<<PIN_ACTIVITY_LED); //activity led off
#endif
#endif
			me->m_timeout.disarm();
			status = Q_HANDLED();
			break;
		}
		default: {
			status = Q_SUPER(&I2CSlave::Started);
			break;
		}
	}
	return status;
}

void I2CSlave::ReceiveCallback(uint8_t highByte, uint8_t lowByte, uint8_t len){
	Evt *evt = new I2CSlaveReceive(highByte, lowByte, len);
	QF::PUBLISH(evt, 0);
}

#if CONFIG_I2C_SLAVE

extern "C" {
	void CONFIG_I2C_SLAVE_HANDLER(void){
		QXK_ISR_ENTRY();
		if( isAddressMatch( CONFIG_I2C_SLAVE_SERCOM ) && slave_busy ){
			prepareNackBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
			prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
		}
		else if(isStopDetectedWIRE( CONFIG_I2C_SLAVE_SERCOM ) ||
		(isAddressMatch( CONFIG_I2C_SLAVE_SERCOM ) && isRestartDetectedWIRE( CONFIG_I2C_SLAVE_SERCOM ) && !isMasterReadOperationWIRE( CONFIG_I2C_SLAVE_SERCOM ))) //Stop or Restart detected
		{
		    FLOW_CONTROL_LOW
			prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
			prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
			
			I2CSlave::ReceiveCallback(high_byte, low_byte, (bytes_received > 0 ? bytes_received - 2 : 0) );
			high_byte = 0;
			low_byte = 0;
			bytes_received = 0;
		}
		else if(isAddressMatch(CONFIG_I2C_SLAVE_SERCOM))  //Address Match
		{
			//otherwise acknowledge right away to clear the interrupt
			prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
			prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
			bytes_received = 0;
			
		}
		else if(isDataReadyWIRE(CONFIG_I2C_SLAVE_SERCOM))
		{
			if (isMasterReadOperationWIRE(CONFIG_I2C_SLAVE_SERCOM))
			{
				uint8_t c;
				uint8_t count = m_outFifo->Read(&c, 1);

				sendDataSlaveWIRE(CONFIG_I2C_SLAVE_SERCOM, (count ? c : 0xff) );
			}
			else { //Received data
				uint8_t c = readDataWIRE(CONFIG_I2C_SLAVE_SERCOM);
				
				bytes_received++;
				
				if(bytes_received == 1) high_byte = c;
				else if(bytes_received == 2) low_byte = c;
				else{
					if ( m_inFifo->Write(&c, 1) ){
						prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
					}
					else {
						prepareNackBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
						bytes_received--;
					}
				}

				prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
			}
		}
		QXK_ISR_EXIT();
	}	
};

#endif
