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

Q_DEFINE_THIS_FILE

using namespace FW;

static Fifo *m_inFifo;
static Fifo *m_outFifo;

static Fifo *m_defaultOutFifo;

volatile bool slave_busy;

volatile uint8_t bytes_received, high_byte, low_byte;

I2CSlave::I2CSlave( Sercom *sercom) :
    QActive((QStateHandler)&I2CSlave::InitialPseudoState), 
    m_id(I2C_SLAVE), m_name("I2C Slave"), m_sercom(sercom) {}

QState I2CSlave::InitialPseudoState(I2CSlave * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(I2C_SLAVE_START_REQ);
    me->subscribe(I2C_SLAVE_STOP_REQ);
      
	me->subscribe(I2C_SLAVE_REQUEST);
	me->subscribe(I2C_SLAVE_RECEIVE);
	me->subscribe(I2C_SLAVE_STOP_CONDITION);
	
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
			
			//TODO: read address pins, increment address if necessary
			initSlaveWIRE( me->m_sercom, CONFIG_I2C_SLAVE_ADDR );
			enableWIRE( me->m_sercom );
			NVIC_ClearPendingIRQ( CONFIG_I2C_SLAVE_IRQn );
			
			slave_busy = false;
			
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SDA, 2);
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SCL, 2);
			
			I2CSlaveStartReq const &req = static_cast<I2CSlaveStartReq const &>(*e);
			m_inFifo = req.getInFifo();
			m_defaultOutFifo = req.getOutFifo();
			m_outFifo = m_defaultOutFifo;
			bytes_received = 0;
			
			m_inFifo->Reset();
			m_outFifo->Reset();
			
			gpio_init(PORTA, PIN_ACTIVITY_LED, 1); //set as output
			
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
			if(req.getRequesterId() == me->m_id){
				status = Q_HANDLED();
			}
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
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
			PORT->Group[PORTA].OUTSET.reg = (1ul<<PIN_ACTIVITY_LED); //activity led on
            status = Q_HANDLED();
            break;
        }
		case I2C_SLAVE_RECEIVE: {
			LOG_EVENT(e);
			
			I2CSlaveReceive const &req = static_cast<I2CSlaveReceive const &>(*e);
			
			if(req.getLen() > 0){
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), req.getLen(), m_inFifo);
				QF::PUBLISH(evt, me);
				status = Q_HANDLED();
			}
			else {
				m_defaultOutFifo->Reset();	
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), 0, m_defaultOutFifo);
				QF::PUBLISH(evt, me);
				status = Q_TRAN(&I2CSlave::Busy);
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
			//TODO: set a timeout on entry to busy state
			LOG_EVENT(e);
			
			slave_busy = true;
			
			//assume default output fifo
			m_outFifo = m_defaultOutFifo;
			
			status = Q_HANDLED();
			
			break;
		}
		case DELEGATE_DATA_READY: {
			DelegateDataReady const &req = static_cast<DelegateDataReady const &>(*e);
			if(req.getRequesterId() == me->m_id){
				if(req.getFifo() != NULL){
					 m_outFifo = req.getFifo();
				}
				status = Q_TRAN(&I2CSlave::Idle);
			}
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			PORT->Group[PORTA].OUTCLR.reg = (1ul<<PIN_ACTIVITY_LED); //activity led off
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
			prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
			prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
			
			I2CSlave::ReceiveCallback(high_byte, low_byte, (bytes_received > 0 ? bytes_received - 2 : 0) );
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
