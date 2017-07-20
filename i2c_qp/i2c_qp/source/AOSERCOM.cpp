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
#include "AOSERCOM.h"
#include "event.h"

#include "SeesawConfig.h"
#include "bsp_sercom.h"
#include "bsp_gpio.h"

Q_DEFINE_THIS_FILE

using namespace FW;

//TODO: we probably will need these for each sercom
static Fifo *m_rxFifo;

AOSERCOM::AOSERCOM( Sercom *sercom, uint8_t id ) :
    QActive((QStateHandler)&AOSERCOM::InitialPseudoState), 
    m_id(id), m_name("SERCOM"), m_sercom(sercom) {}

QState AOSERCOM::InitialPseudoState(AOSERCOM * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(SERCOM_START_REQ);
    me->subscribe(SERCOM_STOP_REQ);
	
	me->subscribe(SERCOM_WRITE_REQ);
      
    return Q_TRAN(&AOSERCOM::Root);
}

QState AOSERCOM::Root(AOSERCOM * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOSERCOM::Stopped);
            break;
        }
		case SERCOM_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOSERCOM::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOSERCOM::Stopped(AOSERCOM * const me, QEvt const * const e) {
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
        case SERCOM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SERCOMStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SERCOM_START_REQ: {
            LOG_EVENT(e);
			
			SercomStartReq const &req = static_cast<SercomStartReq const &>(*e);
			m_rxFifo = req.getRxFifo();
			
			Evt const &r = EVT_CAST(*e);
			Evt *evt = new SERCOMStartCfm(r.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOSERCOM::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOSERCOM::Root);
            break;
        }
    }
    return status;
}

QState AOSERCOM::Started(AOSERCOM * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
			status = Q_TRAN(&AOSERCOM::UART);
			break;
		}
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		
        default: {
            status = Q_SUPER(&AOSERCOM::Root);
            break;
        }
    }
    return status;
}

QState AOSERCOM::UART(AOSERCOM * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			
			//set up UART
			pinPeripheral(CONFIG_SERCOM_UART_PIN_RX, 3);
			pinPeripheral(CONfIG_SERCOM_UART_PIN_TX, 3);

			initUART(me->m_sercom, UART_INT_CLOCK, SAMPLE_RATE_x16, CONFIG_SERCOM_UART_BAUD_RATE);
			initFrame(me->m_sercom, CONFIG_SERCOM_UART_CHAR_SIZE, LSB_FIRST, CONFIG_SERCOM_UART_PARITY, CONFIG_SERCOM_UART_STOP_BIT);
			initPads(me->m_sercom, CONFIG_SERCOM_UART_PAD_TX, CONFIG_SERCOM_UART_PAD_RX);

			enableUART(me->m_sercom);
			
			status = Q_HANDLED();
			break;
		}
		
		case SERCOM_WRITE_REQ: {
			
			SercomWriteReq const &req = static_cast<SercomWriteReq const &>(*e);
			Fifo *source = req.getSource();
			
			uint8_t c;
			while(source->Read(&c, 1)){
				writeDataUART(me->m_sercom, c);
			}
			
			Evt *evt = new DelegateDataReady(req.getRequesterId());
			QF::PUBLISH(evt, me);
			
			status = Q_HANDLED();
			break;	
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			break;
		}
		
		default: {
			status = Q_SUPER(&AOSERCOM::Started);
			break;
		}
	}
	return status;
}

QState AOSERCOM::SPI(AOSERCOM * const me, QEvt const * const e) {
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
		
		default: {
			status = Q_SUPER(&AOSERCOM::Started);
			break;
		}
	}
	return status;
}

extern "C" {
	void SERCOM5_Handler(void){
		QXK_ISR_ENTRY();
		if (availableDataUART( SERCOM5 )) {
			uint8_t c = readDataUART( SERCOM5 );
			m_rxFifo->Write(&c, 1);
			//TODO: post interrupt
		}

		if (isUARTError( SERCOM5 )) {
			acknowledgeUARTError( SERCOM5 );
			// TODO: if (sercom->isBufferOverflowErrorUART()) ....
			// TODO: if (sercom->isFrameErrorUART()) ....
			// TODO: if (sercom->isParityErrorUART()) ....
			clearStatusUART( SERCOM5 );
		}
		QXK_ISR_EXIT();
	}
};

/*
QState AOSERCOM::stateName(AOSERCOM * const me, QEvt const * const e) {
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
		
		default: {
			status = Q_SUPER(&AOSERCOM::Root);
			break;
		}
	}
	return status;
}*/
