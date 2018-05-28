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
#include "SPISlave.h"
#include "event.h"
#include "bsp_gpio.h"

#include "bsp_sercom.h"
#include "SeesawConfig.h"
#include "PinMap.h"
#include "bsp_nvmctrl.h"
#include "bsp_dma.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#if CONFIG_SPI_SLAVE

static Fifo *m_inFifo;
static Fifo *m_outFifo;

static Fifo *m_defaultOutFifo;

static volatile bool slave_busy;

static uint8_t bytes_received, high_byte, low_byte;

SPISlave::SPISlave( Sercom *sercom) :
    QActive((QStateHandler)&SPISlave::InitialPseudoState), 
    m_id(SPI_SLAVE), m_name("SPI Slave"), m_sercom(sercom) {}

QState SPISlave::InitialPseudoState(SPISlave * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(SPI_SLAVE_START_REQ);
    me->subscribe(SPI_SLAVE_STOP_REQ);
      
	me->subscribe(SPI_SLAVE_REQUEST);
	me->subscribe(SPI_SLAVE_RECEIVE);

	me->subscribe(DELEGATE_DATA_READY);
	  
    return Q_TRAN(&SPISlave::Root);
}

QState SPISlave::Root(SPISlave * const me, QEvt const * const e) {
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
            status = Q_TRAN(&SPISlave::Stopped);
            break;
        }
		case SPI_SLAVE_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&SPISlave::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState SPISlave::Stopped(SPISlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			disableSPI(me->m_sercom);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case SPI_SLAVE_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SPISlaveStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SPI_SLAVE_START_REQ: {
            LOG_EVENT(e);
			
			//3 is sercom-alt TODO: put this in config
			pinPeripheral(CONFIG_SPI_SLAVE_PIN_MOSI, 3);
			pinPeripheral(CONFIG_SPI_SLAVE_PIN_MISO, 3);
			pinPeripheral(CONFIG_SPI_SLAVE_PIN_SCK, 3);
			pinPeripheral(CONFIG_SPI_SLAVE_PIN_SS, 3);

			initSPISlave( me->m_sercom, CONFIG_SPI_SLAVE_PAD_TX, CONFIG_SPI_SLAVE_PAD_RX, CONFIG_SPI_SLAVE_CHAR_SIZE,CONFIG_SPI_SLAVE_DATA_ORDER);
			setClockModeSPI( me->m_sercom, SERCOM_SPI_MODE_3);
			NVIC_ClearPendingIRQ(CONFIG_SPI_SLAVE_IRQn);
			
			//enable transaction complete interrupt
			me->m_sercom->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
			
			SPISlaveStartReq const &req = static_cast<SPISlaveStartReq const &>(*e);
			m_inFifo = req.getInFifo();
			m_defaultOutFifo = req.getOutFifo();
			m_outFifo = m_defaultOutFifo;
			bytes_received = 0;
			slave_busy = false;
			
			m_inFifo->Reset();
			m_outFifo->Reset();

			gpio_init(PORTA, CONFIG_INTERRUPT_PIN, 1); //set as output
			gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 0); //write low

            dmac_init();
            dmac_alloc(0);

            dmac_set_action(0, DMA_TRIGGER_ACTON_BEAT);
            dmac_set_trigger(0, SERCOM3_DMAC_ID_RX);

            dmac_set_descriptor(
              0,
              (void *)&me->m_sercom->SPI.DATA.reg,
              (void *)m_inFifo->GetAddr(0),
              64,
              DMA_BEAT_SIZE_BYTE,
              false,
              true);

            dmac_alloc(1);
            dmac_set_action(1, DMA_TRIGGER_ACTON_BEAT);
            dmac_set_trigger(1, SERCOM3_DMAC_ID_TX);

            //gpio_init(PORTA, PIN_ACTIVITY_LED, 1); //set as output

			Evt *evt = new SPISlaveStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&SPISlave::Started);
            break;
        }
        default: {
            status = Q_SUPER(&SPISlave::Root);
            break;
        }
    }
    return status;
}

QState SPISlave::Started(SPISlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            enableSPI(me->m_sercom);
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
            status = Q_TRAN(&SPISlave::Idle);
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		case SPI_SLAVE_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new SPISlaveStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(SPISlave::Stopped);
			break;
		}
		case DELEGATE_DATA_READY:{
			LOG_EVENT(e);
			DelegateDataReady const &req = static_cast<DelegateDataReady const &>(*e);
			
			//host may have cancelled transaction
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
            status = Q_SUPER(&SPISlave::Root);
            break;
        }
    }
    return status;
}

QState SPISlave::Idle(SPISlave * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);

            dmac_start(0);
            gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 1); //write high

            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
			//PORT->Group[PORTA].OUTSET.reg = (1ul<<PIN_ACTIVITY_LED); //activity led on
            status = Q_HANDLED();
            break;
        }
		case SPI_SLAVE_RECEIVE: {
			LOG_EVENT(e);
			
			SPISlaveReceive const &req = static_cast<SPISlaveReceive const &>(*e);
			
			if((req.getLowByte() > 0 || req.getHighByte() > 0) && req.getLen() > 0){
				//write data, no response needed
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), req.getLen(), m_inFifo);
				QF::PUBLISH(evt, me);

				dmac_start(0);
				gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 1); //signal we are now ready to accept commands again

				status = Q_HANDLED();
			}
			else if(req.getLowByte() > 0 || req.getHighByte() > 0) {
				//read data, go into busy state and wait for a response
				m_defaultOutFifo->Reset();	
				Evt *evt = new DelegateProcessCommand(me->m_id, req.getHighByte(), req.getLowByte(), 0, m_defaultOutFifo);
				QF::PUBLISH(evt, me);
				status = Q_TRAN(&SPISlave::Busy);
			}
			else{
				dmac_start(0);
				gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 1); //signal we are now ready to accept commands again
				status = Q_HANDLED();
			}
			
			break;
		}
        default: {
            status = Q_SUPER(&SPISlave::Started);
            break;
        }
    }
    return status;
}

QState SPISlave::Busy(SPISlave * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			
			//dmac_abort(0);
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
	            dmac_set_descriptor(
	              1,
	              (void *)m_outFifo->GetAddr(0),
	              (void *)&me->m_sercom->SPI.DATA.reg,
	              64,
	              DMA_BEAT_SIZE_BYTE,
	              true,
	              false);

                dmac_start(1);

                gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 1); //write high

                status = Q_HANDLED();
				
			}
			else
				status = Q_UNHANDLED();
			break;
		}
		case SPI_SLAVE_RECEIVE: {
			LOG_EVENT(e);
			status = Q_TRAN(&SPISlave::Idle);
			break;
		}
		
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			slave_busy = false;
			//PORT->Group[PORTA].OUTCLR.reg = (1ul<<PIN_ACTIVITY_LED); //activity led off
			status = Q_HANDLED();
			break;
		}
		default: {
			status = Q_SUPER(&SPISlave::Started);
			break;
		}
	}
	return status;
}

void SPISlave::ReceiveCallback(uint8_t highByte, uint8_t lowByte, uint8_t len){
	Evt *evt = new SPISlaveReceive(highByte, lowByte, len);
	QF::PUBLISH(evt, 0);
}

extern "C" {
	void CONFIG_SPI_SLAVE_HANDLER(void){
		QXK_ISR_ENTRY();
		
		if(CONFIG_SPI_SLAVE_SERCOM->SPI.INTFLAG.bit.TXC){

			gpio_write(PORTA, CONFIG_INTERRUPT_PIN, 0); //signal to the host to stop sending data

			CONFIG_SPI_SLAVE_SERCOM->SPI.INTFLAG.bit.TXC = 1;

			dmac_abort(1);
            dmac_abort(0);

            if(!slave_busy)
            	bytes_received = dmac_get_transfer_count(0);

			m_inFifo->Reset();
			m_inFifo->SetIndex(bytes_received);
			m_inFifo->Read(&high_byte, 1);
			m_inFifo->Read(&low_byte, 1);

			//the transaction is complete, send to the system to process
			SPISlave::ReceiveCallback(high_byte, low_byte, (bytes_received > 1 ? bytes_received - 2 : 0) );
			high_byte = 0;
			low_byte = 0;

			bytes_received = 0;
			dmac_start(0);
		}
		
		QXK_ISR_EXIT();
	}	
};

#endif
