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
#include "System.h"
#include "event.h"

#include "RegisterMap.h"

Q_DEFINE_THIS_FILE

using namespace FW;

System::System() :
    QActive((QStateHandler)&System::InitialPseudoState), 
    m_id(SYSTEM), m_name("SYSTEM")
    //,m_testTimer(this, SYSTEM_TEST_TIMER)
#if CONFIG_I2C_SLAVE
    ,m_I2CSlaveOutFifo(m_I2CSlaveOutFifoStor, I2C_SLAVE_OUT_FIFO_ORDER),
    m_I2CSlaveInFifo(m_I2CSlaveInFifoStor, I2C_SLAVE_IN_FIFO_ORDER) 
#endif

#if CONFIG_SPI_SLAVE
	,m_SPISlaveOutFifo(m_SPISlaveOutFifoStor, SPI_SLAVE_OUT_FIFO_ORDER),
	m_SPISlaveInFifo(m_SPISlaveInFifoStor, SPI_SLAVE_IN_FIFO_ORDER)
#endif

#if CONFIG_USB
	,m_USBOutFifo(m_USBOutFifoStor, USB_OUT_FIFO_ORDER),
	m_USBInFifo(m_USBInFifoStor, USB_IN_FIFO_ORDER)
#endif

#if CONFIG_SERCOM0
	,m_sercom0RxFifo(m_sercom0RxFifoStor, SERCOM_FIFO_ORDER)
#endif

#if CONFIG_SERCOM1
	,m_sercom1RxFifo(m_sercom1RxFifoStor, SERCOM_FIFO_ORDER)
#endif

#if CONFIG_SERCOM2
	,m_sercom2RxFifo(m_sercom2RxFifoStor, SERCOM_FIFO_ORDER)
#endif

#if CONFIG_SERCOM5
	,m_sercom5RxFifo(m_sercom5RxFifoStor, SERCOM_FIFO_ORDER)
#endif

#if CONFIG_DAP
	,m_DAPRxFifo(m_DAPRxFifoStor, DAP_FIFO_ORDER)
#endif
	{}

QState System::InitialPseudoState(System * const me, QEvt const * const e) {
    (void)e;
    //me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(SYSTEM_START_REQ);
    me->subscribe(SYSTEM_STOP_REQ);
    //me->subscribe(SYSTEM_TEST_TIMER);
    me->subscribe(SYSTEM_DONE);
    me->subscribe(SYSTEM_FAIL);
	
#if CONFIG_I2C_SLAVE
	me->subscribe(I2C_SLAVE_START_CFM);
	me->subscribe(I2C_SLAVE_STOP_CFM);
#endif

#if CONFIG_SPI_SLAVE
	me->subscribe(SPI_SLAVE_START_CFM);
	me->subscribe(SPI_SLAVE_STOP_CFM);
#endif

#if CONFIG_SERCOM0 || CONFIG_SERCOM1 || CONFIG_SERCOM2 || CONFIG_SERCOM3 || CONFIG_SERCOM4 || CONFIG_SERCOM5
	me->subscribe(SERCOM_START_CFM);
	me->subscribe(SERCOM_STOP_CFM);
#endif
	
#if CONFIG_ADC
	me->subscribe(ADC_START_CFM);
	me->subscribe(ADC_STOP_CFM);
#endif
	
#if CONFIG_TIMER
	me->subscribe(TIMER_START_CFM);
	me->subscribe(TIMER_STOP_CFM);
#endif	

#if CONFIG_DAC
	me->subscribe(DAC_START_CFM);
	me->subscribe(DAC_STOP_CFM);
#endif
	
	me->subscribe(DELEGATE_START_CFM);
	me->subscribe(DELEGATE_STOP_CFM);
	
#if CONFIG_INTERRUPT
	me->subscribe(INTERRUPT_START_CFM);
	me->subscribe(INTERRUPT_STOP_CFM);
#endif
	
#if CONFIG_DAP
	me->subscribe(DAP_START_CFM);
	me->subscribe(DAP_STOP_CFM);
#endif

#if CONFIG_NEOPIXEL
	me->subscribe(NEOPIXEL_START_CFM);
	me->subscribe(NEOPIXEL_STOP_CFM);
#endif

#if CONFIG_USB
	me->subscribe(USB_START_CFM);
	me->subscribe(USB_STOP_CFM);
#endif
      
    return Q_TRAN(&System::Root);
}

QState System::Root(System * const me, QEvt const * const e) {
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
            status = Q_TRAN(&System::Stopped);
            break;
        }
		case SYSTEM_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&System::Stopping);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState System::Stopped(System * const me, QEvt const * const e) {
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
        case SYSTEM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SystemStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_START_REQ: {
            LOG_EVENT(e);
            status = Q_TRAN(&System::Starting);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Stopping(System * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			me->m_cfmCount = 0;			
						
			Evt *evt = new Evt(DELEGATE_STOP_REQ);
			QF::PUBLISH(evt, me);
			
#if CONFIG_I2C_SLAVE
			evt = new Evt(I2C_SLAVE_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SPI_SLAVE
			evt = new Evt(SPI_SLAVE_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_DAC			
			evt = new Evt(DAC_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_ADC			
			evt = new Evt(ADC_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_TIMER
			evt = new Evt(TIMER_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_INTERRUPT
			evt = new Evt(INTERRUPT_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

//TODO: this should be broken out target specific sercoms
#if CONFIG_SERCOM0
			evt = new Evt(SERCOM_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM1
			evt = new Evt(SERCOM_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM2
			evt = new Evt(SERCOM_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM5			
			evt = new Evt(SERCOM_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_DAP
			evt = new Evt(DAP_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_NEOPIXEL
			evt = new Evt(NEOPIXEL_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_USB
			evt = new Evt(USB_STOP_REQ);
			QF::PUBLISH(evt, me);
#endif
			
			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			break;
		}
		
		case ADC_STOP_CFM:
		case DAC_STOP_CFM:
		case TIMER_STOP_CFM:
		case SERCOM_STOP_CFM:
		case I2C_SLAVE_STOP_CFM:
		case SPI_SLAVE_STOP_CFM:
		case INTERRUPT_STOP_CFM:
		case DAP_STOP_CFM:
		case NEOPIXEL_STOP_CFM:
		case USB_STOP_CFM:
		case DELEGATE_STOP_CFM: {
			LOG_EVENT(e);
			me->HandleCfm(ERROR_EVT_CAST(*e), CONFIG_NUM_AO);
			status = Q_HANDLED();
			break;
		}
		
		case SYSTEM_FAIL:
			Q_ASSERT(0);
			break;
		case SYSTEM_DONE: {
			LOG_EVENT(e);
			Evt *evt = new SystemStartReq(me->m_nextSequence++);
			me->postLIFO(evt);
			status = Q_TRAN(&System::Stopped);
			break;
		}
		default: {
			status = Q_SUPER(&System::Root);
			break;
		}
	}
	return status;
}

QState System::Starting(System * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			me->m_cfmCount = 0;	
			
			//start objects
			Evt *evt = new Evt(DELEGATE_START_REQ);
			QF::PUBLISH(evt, me);
			
#if CONFIG_I2C_SLAVE
			evt = new I2CSlaveStartReq(&me->m_I2CSlaveOutFifo, &me->m_I2CSlaveInFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SPI_SLAVE
			evt = new SPISlaveStartReq(&me->m_SPISlaveOutFifo, &me->m_SPISlaveInFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_DAC		
			evt = new Evt(DAC_START_REQ);
			QF::PUBLISH(evt, me);
#endif
			
#if CONFIG_ADC
			evt = new Evt(ADC_START_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_TIMER
			evt = new Evt(TIMER_START_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_INTERRUPT
			evt = new Evt(INTERRUPT_START_REQ);
			QF::PUBLISH(evt, me);
#endif

//TODO: this should be broken out target specific sercoms
#if CONFIG_SERCOM0
			evt = new SercomStartReq(&me->m_sercom0RxFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM1
			evt = new SercomStartReq(&me->m_sercom1RxFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM2
			evt = new SercomStartReq(&me->m_sercom2RxFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_SERCOM5
			evt = new SercomStartReq(&me->m_sercom5RxFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_DAP
			evt = new DAPStartReq(&me->m_DAPRxFifo);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_NEOPIXEL
			evt = new Evt(NEOPIXEL_START_REQ);
			QF::PUBLISH(evt, me);
#endif

#if CONFIG_USB
			evt = new USBStartReq(&me->m_USBOutFifo, &me->m_USBInFifo);
			QF::PUBLISH(evt, me);
#endif

			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			//me->m_stateTimer.disarm();
			status = Q_HANDLED();
			break;
		}
		
		case ADC_START_CFM:
		case TIMER_START_CFM:
		case DAC_START_CFM:
		case SERCOM_START_CFM:
		case I2C_SLAVE_START_CFM:
		case SPI_SLAVE_START_CFM:
		case INTERRUPT_START_CFM:
		case DAP_START_CFM:
		case NEOPIXEL_START_CFM:
		case USB_START_CFM:
		case DELEGATE_START_CFM: {
			LOG_EVENT(e);
			me->HandleCfm(ERROR_EVT_CAST(*e), CONFIG_NUM_AO);
			status = Q_HANDLED();
			break;
		}

		case SYSTEM_FAIL:
		//TODO:
		
		case SYSTEM_DONE: {
			LOG_EVENT(e);
			Evt *evt = new SystemStartCfm(me->m_savedInSeq, ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(&System::Started);
			break;
		}
		default: {
			status = Q_SUPER(&System::Root);
			break;
		}
	}
	return status;
}

QState System::Started(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            //me->m_testTimer.armX(2000, 2000);
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            //me->m_testTimer.disarm();
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_TEST_TIMER: {
			//when the timer goes off, this will get hit. Log the event and publish another event
            LOG_EVENT(e);
			
			/*
			const uint8_t pinmodeData = 0x82;
			me->m_I2CSlaveInFifo.Write(&pinmodeData, 1);
            Evt *evt = new DelegateProcessCommand(me->m_id, SEESAW_GPIO_BASE, SEESAW_GPIO_PINMODE_CMD, 1, &me->m_I2CSlaveInFifo);
            QF::PUBLISH(evt, me);
			
			const uint8_t pinToggleData = 0x02;
			me->m_I2CSlaveInFifo.Write(&pinToggleData, 1);
			evt = new DelegateProcessCommand(me->m_id, SEESAW_GPIO_BASE, SEESAW_GPIO_TOGGLE_CMD, 1, &me->m_I2CSlaveInFifo);
			QF::PUBLISH(evt, me);
			*/
			
			status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

void System::HandleCfm(ErrorEvt const &e, uint8_t expectedCnt) {
	if (e.GetError() == ERROR_SUCCESS) {
		// TODO - Compare seqeuence number.
		if(++m_cfmCount == expectedCnt) {
			Evt *evt = new Evt(SYSTEM_DONE);
			postLIFO(evt);
		}
		} else {
		Evt *evt = new SystemFail(e.GetError(), e.GetReason());
		postLIFO(evt);
	}
}

