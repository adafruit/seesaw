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
#include "Delegate.h"
#include "event.h"

#include "RegisterMap.h"
#include "PinMap.h"

#include "bsp_gpio.h"

Q_DEFINE_THIS_FILE

using namespace FW;

Delegate::Delegate() :
    QActive((QStateHandler)&Delegate::InitialPseudoState), 
    m_id(DELEGATE), m_name("Delegate") {}

QState Delegate::InitialPseudoState(Delegate * const me, QEvt const * const e) {
    (void)e;
	
    me->subscribe(DELEGATE_START_REQ);
	me->subscribe(DELEGATE_STOP_REQ);
    
	me->subscribe(DELEGATE_PROCESS_COMMAND);
	
    return Q_TRAN(&Delegate::Root);
}

QState Delegate::Root(Delegate * const me, QEvt const * const e) {
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
            status = Q_TRAN(&Delegate::Stopped);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState Delegate::Stopped(Delegate * const me, QEvt const * const e) {
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
		case DELEGATE_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new DelegateStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_HANDLED();
			break;
		}
		case DELEGATE_START_REQ: {
			LOG_EVENT(e);
			DelegateStartReq const &req = static_cast<DelegateStartReq const &>(*e);
			me->m_I2CSlaveInFifo = req.getI2CSlaveInFifo();
			me->m_I2CSlaveOutFifo = req.getI2CSlaveOutFifo();
			
			Evt *evt = new DelegateStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&Delegate::Started);
			break;
		}
		default: {
			status = Q_SUPER(&Delegate::Root);
			break;
		}
	}
	return status;
}

QState Delegate::Started(Delegate * const me, QEvt const * const e) {
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
		case DELEGATE_PROCESS_COMMAND: {
			
			DelegateProcessCommand const &req = static_cast<DelegateProcessCommand const &>(*e);
			uint8_t highByte = req.getHighByte();
			uint8_t lowByte = req.getLowByte();
			
			switch(highByte){
				
				//We don't have a separate AO to handle STATUS or GPIO stuff since it's simple and a waste of resources
				case SEESAW_STATUS_BASE: {
					break;
				}
				case SEESAW_GPIO_BASE: {
					
					Fifo *fifo = req.getFifo();
					uint8_t dataByte;
					fifo->Read(&dataByte, 1);
					switch(lowByte){
						case SEESAW_GPIO_PINMODE_CMD: {
							_PinDescription pin = g_APinDescription[SEESAW_GPIO_GET_PINMODE_PIN(dataByte)];
							gpio_init(pin.ulPort, pin.ulPin, SEESAW_GPIO_GET_PINMODE_MODE(dataByte));
							break;
						}
						case SEESAW_GPIO_TOGGLE_CMD: {
							_PinDescription pin = g_APinDescription[SEESAW_GPIO_GET_TOGGLE_PIN(dataByte)];
							gpio_toggle(pin.ulPort, pin.ulPin);
							break;
						}
						//TODO: fill in GPIO read, write commands following this format
					}
					break;
				}
				default:
					break;
			}
			status = Q_HANDLED();
			
			break;
		}
        default: {
            status = Q_SUPER(&Delegate::Root);
            break;
        }
    }
    return status;
}