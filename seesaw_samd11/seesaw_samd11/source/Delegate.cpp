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
#include "SeesawConfig.h"
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
			Evt const &req = EVT_CAST(*e);
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
			uint8_t len = req.getLen();
			
			if(!len){
				//we are reading
				switch(highByte){
					
					//We don't have a separate AO to handle STATUS or GPIO stuff since it's simple and a waste of resources
					case SEESAW_STATUS_BASE: {
						Fifo *fifo = req.getFifo();
						switch(lowByte){
							case SEESAW_STATUS_VERSION:{
								uint8_t r = CONFIG_VERSION;
								fifo->Write(&r, 1);
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
						}
						break;
					}
					case SEESAW_SERCOM0_BASE:
					case SEESAW_SERCOM1_BASE:
					case SEESAW_SERCOM2_BASE:
					case SEESAW_SERCOM3_BASE:
					case SEESAW_SERCOM4_BASE:
					case SEESAW_SERCOM5_BASE:{
						switch(lowByte){
							//TODO: fix for more sercoms
							case SEESAW_SERCOM_STATUS:
							case SEESAW_SERCOM_INTEN:
							case SEESAW_SERCOM_BAUD:{
								Evt *evt = new SercomReadRegReq(req.getRequesterId(), lowByte, req.getFifo());
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_SERCOM_DATA:{
								Evt *evt = new SercomReadDataReq(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
							default: {
								__BKPT();
								Q_ASSERT(0);
							}
						}
						break;
					}
					default:
						//Unrecognized command or unreadable register. Do nothing.
						Evt *evt = new DelegateDataReady(req.getRequesterId());
						QF::PUBLISH(evt, me);
						break;
				}
			}
			
			else{
				//we are writing
				switch(highByte){
				
					//We don't have a separate AO to handle STATUS or GPIO stuff since it's simple and a waste of resources
					case SEESAW_STATUS_BASE: {
						switch(lowByte){
							case SEESAW_STATUS_SWRST:{
								Evt *evt = new Evt(SYSTEM_STOP_REQ);
								QF::PUBLISH(evt, me);
								break;
							}
						}
						break;
					}
					case SEESAW_GPIO_BASE: {
					
						Fifo *fifo = req.getFifo();
						uint8_t dataByte;
						fifo->Read(&dataByte, 1);
						len--;
						
						//read any extra bytes and discard
						while(len > 0){
							uint8_t dummy;
							fifo->Read(&dummy, 1);
							len--;
						}
						
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
							//TODO: fill in more stuff
						}
						break;
					}
					case SEESAW_SERCOM0_BASE:
					case SEESAW_SERCOM1_BASE:
					case SEESAW_SERCOM2_BASE:
					case SEESAW_SERCOM3_BASE:
					case SEESAW_SERCOM4_BASE:
					case SEESAW_SERCOM5_BASE:{
						switch(lowByte){
							//TODO: fix for more sercoms
							case SEESAW_SERCOM_STATUS:
							case SEESAW_SERCOM_INTEN:
							case SEESAW_SERCOM_BAUD:{
								Fifo *fifo = req.getFifo();
								uint8_t dataByte;
								fifo->Read(&dataByte, 1);
								len--;
								
								//read any extra bytes and discard
								me->discard(fifo, len);
								
								Evt *evt = new SercomWriteRegReq(lowByte, dataByte);
								QF::PUBLISH(evt, me);
								
								//ack immediately
								evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_SERCOM_DATA:{
								//TODO: this should take in number of bytes to write
								Evt *evt = new SercomWriteDataReq(req.getRequesterId(), req.getFifo());
								QF::PUBLISH(evt, me);
								break;
							}
						}
					}
					case SEESAW_TIMER_BASE:{
						switch(lowByte){
							case SEESAW_TIMER_PWM: {
								Fifo *fifo = req.getFifo();
								uint8_t dataBytes[2];
								fifo->Read(dataBytes, 2);
								len -= 2;
								
								me->discard(fifo, len);
								
								Evt *evt = new TimerWritePWM(dataBytes[0], dataBytes[1]);
								QF::PUBLISH(evt, me);
								
								break;
							}
						}
					}
					default:
						break;
				}
			}
			status = Q_HANDLED();
			
			break;
		}
		case DELEGATE_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new DelegateStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(Delegate::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&Delegate::Root);
            break;
        }
    }
    return status;
}

void Delegate::discard(Fifo *fifo, uint8_t len)
{
	//read any extra bytes and discard
	while(len > 0){
		uint8_t dummy;
		fifo->Read(&dummy, 1);
		len--;
	}
}