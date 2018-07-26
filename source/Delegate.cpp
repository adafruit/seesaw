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

#include "bsp_nvmctrl.h"
#include "bsp_sercom.h"

Q_DEFINE_THIS_FILE

using namespace FW;

volatile uint32_t Delegate::m_inten = 0;
volatile uint32_t Delegate::m_intflag = 0;

Delegate::Delegate() :
    QActive((QStateHandler)&Delegate::InitialPseudoState), 
    m_id(DELEGATE), m_name("Delegate") {}

QState Delegate::InitialPseudoState(Delegate * const me, QEvt const * const e) {
    (void)e;
	
    me->subscribe(DELEGATE_START_REQ);
	me->subscribe(DELEGATE_STOP_REQ);
    
	me->subscribe(DELEGATE_PROCESS_COMMAND);
	me->subscribe(GPIO_INTERRUPT_RECEIVED);
	
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
			Delegate::m_inten = 0;
			Delegate::m_intflag = 0;
			
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new DelegateStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&Delegate::Started);
			break;
		}
		case GPIO_INTERRUPT_RECEIVED: {
			//ignore
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
			
#ifdef ENABLE_LOGGING
            PRINT("DELEGATE_PROCESS_COMMAND: (0x%x, 0x%x) %i\n", highByte, lowByte, len);
#endif

			if(!len){
				//we are reading
				switch(highByte){
					
					//We don't have a separate AO to handle STATUS or GPIO stuff since it's simple and a waste of resources
					case SEESAW_STATUS_BASE: {
						Fifo *fifo = req.getFifo();
						switch(lowByte){
							case SEESAW_STATUS_HW_ID:{
								uint8_t r = SEESAW_STATUS_HW_ID_CODE;
								fifo->Write(&r, 1);
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_STATUS_VERSION: {
								uint8_t ret[4];
								me->break32Bit(CONFIG_VERSION, ret);
								fifo->Write(ret, 4);
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_STATUS_OPTIONS: {
								uint32_t data = CONFIG_OPTIONS;
								uint8_t ret[4];
								me->break32Bit(data, ret);
								fifo->Write(ret, 4);
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
							default:
								//Unrecognized command or unreadable register. Do nothing.
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
						}
						break;
					}
					case SEESAW_GPIO_BASE: {
						Fifo *fifo = req.getFifo();
						switch(lowByte){
							case SEESAW_GPIO_BULK: {
								uint32_t data = gpio_read_bulk(PORTA);
								uint8_t ret[4];
								me->break32Bit(data, ret);
								fifo->Write(ret, 4);
#ifdef HAS_PORTB
								data = gpio_read_bulk(PORTB);
								me->break32Bit(data, ret);
                                fifo->Write(ret, 4);
#endif
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								
								//clear interrupt if interrupts are enabled
								if(Delegate::m_inten > 0 && Delegate::m_intflag > 0){
									Delegate::m_intflag = 0;
									Evt *evt = new InterruptClearReq( SEESAW_INTERRUPT_GPIO );
									QF::PUBLISH(evt, me);
								}
								
								break;
							}
							case SEESAW_GPIO_INTFLAG: {
								uint8_t ret[4];
								me->break32Bit(Delegate::m_intflag, ret);
								fifo->Write(ret, 4);
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								
								//clear interrupt if interrupts are enabled
								if(Delegate::m_inten > 0){
									Delegate::m_intflag = 0;
									Evt *evt = new InterruptClearReq( SEESAW_INTERRUPT_GPIO );
									QF::PUBLISH(evt, me);
								}
								break;
							}
							default:
								//unrecognized command
								Evt *evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
						}
						break;
					}
#if CONFIG_ADC
					case SEESAW_ADC_BASE: {
						switch(lowByte){
							case SEESAW_ADC_WINMODE:
							case SEESAW_ADC_INTEN:
							case SEESAW_ADC_INTENCLR:
#if CONFIG_ADC_INPUT_0
							case SEESAW_ADC_CHANNEL_0:
#endif
#if CONFIG_ADC_INPUT_1
							case SEESAW_ADC_CHANNEL_1:
#endif
#if CONFIG_ADC_INPUT_2
							case SEESAW_ADC_CHANNEL_2:
#endif
#if CONFIG_ADC_INPUT_3
							case SEESAW_ADC_CHANNEL_3:
#endif
#if CONFIG_ADC_INPUT_4
							case SEESAW_ADC_CHANNEL_4:
#endif
#if CONFIG_ADC_INPUT_5
                            case SEESAW_ADC_CHANNEL_5:
#endif
#if CONFIG_ADC_INPUT_6
                            case SEESAW_ADC_CHANNEL_6:
#endif
#if CONFIG_ADC_INPUT_7
                            case SEESAW_ADC_CHANNEL_7:
#endif
                            default: {
							    Evt *evt = new ADCReadRegReq(req.getRequesterId(), lowByte, req.getFifo());
                                QF::PUBLISH(evt, me);
                                break;
							}
						}
						break;
					}
#endif //ADC

#if CONFIG_TOUCH
                    case SEESAW_TOUCH_BASE: {
                        switch(lowByte){
                            default: {
                                Evt *evt = new TouchReadRegReq(req.getRequesterId(), lowByte, req.getFifo());
                                QF::PUBLISH(evt, me);
                                break;
                            }
                        }
                        break;
                    }
#endif //TOUCH

#if CONFIG_SERCOM0 || CONFIG_SERCOM1 || CONFIG_SERCOM2 || CONFIG_SERCOM3 || CONFIG_SERCOM4 || CONFIG_SERCOM5 
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
#endif //SERCOM

#if CONFIG_DAP
					case SEESAW_DAP_BASE:{
						Evt *evt = new DAPRead(req.getRequesterId());
						QF::PUBLISH(evt, me);
						break;
					}

#endif //DAP

#if CONFIG_EEPROM
					case SEESAW_EEPROM_BASE:{
						Fifo *fifo = req.getFifo();
						uint8_t r = eeprom_read_byte(lowByte);
						fifo->Write(&r, 1);

						Evt *evt = new DelegateDataReady(req.getRequesterId());
						QF::PUBLISH(evt, me);
						break;
					}
#endif

#if CONFIG_NEOPIXEL					
					case SEESAW_NEOPIXEL_BASE:{
						switch(lowByte){
							case SEESAW_NEOPIXEL_SHOW:{
								Evt *evt = new Evt(NEOPIXEL_SHOW_REQ);
								QF::PUBLISH(evt, me);
								
								evt = new DelegateDataReady(req.getRequesterId());
								QF::PUBLISH(evt, me);
								break;
							}
						}
						break;
					}
#endif

#if CONFIG_KEYPAD
					case SEESAW_KEYPAD_BASE:{
						Evt *evt = new KeypadReadRegReq(req.getRequesterId(), lowByte, req.getFifo());
						QF::PUBLISH(evt, me);
						break;
					}
#endif
					
					
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
						switch(lowByte){
							case SEESAW_GPIO_DIRSET_BULK: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];

								gpio_dirset_bulk(PORTA, combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
								if(len > 0){
								    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_dirset_bulk(PORTB, combined & CONFIG_GPIO_B_MASK);
								}
#endif
								break;
							}
							case SEESAW_GPIO_DIRCLR_BULK: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								gpio_dirclr_bulk(PORTA, combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
                                if(len > 0){
                                    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_dirclr_bulk(PORTB, combined & CONFIG_GPIO_B_MASK);
                                }
#endif
								
								break;
							}
							case SEESAW_GPIO_BULK_SET: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								gpio_outset_bulk(PORTA, combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
                                if(len > 0){
                                    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_outset_bulk(PORTB, combined & CONFIG_GPIO_B_MASK);
                                }
#endif
								break;
							}
							case SEESAW_GPIO_BULK_CLR: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								gpio_outclr_bulk(PORTA, combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
                                if(len > 0){
                                    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_outclr_bulk(PORTB, combined & CONFIG_GPIO_B_MASK);
                                }
#endif
								
								break;
							}
							case SEESAW_GPIO_INTENSET: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								Delegate::m_inten |= (combined  & CONFIG_GPIO_A_MASK);
								break;
							}
							case SEESAW_GPIO_INTENCLR: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								combined &= CONFIG_GPIO_A_MASK;
								Delegate::m_inten &= !combined;
								break;
							}
							case SEESAW_GPIO_PULLENSET: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								gpio_pullenset_bulk(combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
                                if(len > 0){
                                    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_pullenset_bulk(combined & CONFIG_GPIO_B_MASK, PORTB);
                                }
#endif
								break;
							}
							case SEESAW_GPIO_PULLENCLR: {
								uint8_t pins[4];
								fifo->Read(pins, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
								gpio_pullenclr_bulk(combined & CONFIG_GPIO_A_MASK);
#ifdef HAS_PORTB
                                if(len > 0){
                                    fifo->Read(pins, 4);
                                    len-=4;

                                    uint32_t combined = ((uint32_t)pins[0] << 24) | ((uint32_t)pins[1] << 16) | ((uint32_t)pins[2] << 8) | (uint32_t)pins[3];
                                    gpio_pullenclr_bulk(combined & CONFIG_GPIO_B_MASK, PORTB);
                                }
#endif
								break;
							}
						}
						//discard any extra data
						me->discard(fifo, len);
						break;
					}
#if CONFIG_ADC
					case SEESAW_ADC_BASE: {
						switch(lowByte){
							//these take 1 byte of data
							case SEESAW_ADC_WINMODE:
							case SEESAW_ADC_INTEN:
							case SEESAW_ADC_INTENCLR:{
								Fifo *fifo = req.getFifo();
								uint8_t dataByte;
								fifo->Read(&dataByte, 1);
								len--;
								
								//read any extra bytes and discard
								me->discard(fifo, len);
								
								Evt *evt = new ADCWriteRegReq(lowByte, dataByte);
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_ADC_WINTHRESH:{
								Fifo *fifo = req.getFifo();
								uint8_t data[4];
								fifo->Read(data, 4);
								len-=4;
								
								//read any extra bytes and discard
								me->discard(fifo, len);
								
								uint16_t ht = ((uint16_t)data[0] << 8) | data[1];
								uint16_t lt = ((uint16_t)data[2] << 8) | data[3];
								
								Evt *evt = new ADCWriteWinmonThresh(ht, lt);
								QF::PUBLISH(evt, me);
								break;
							}
						}
						break;
					}
#endif // ADC
					
#if ( CONFIG_SERCOM0 | CONFIG_SERCOM1 | CONFIG_SERCOM2 | CONFIG_SERCOM3 | CONFIG_SERCOM4 | CONFIG_SERCOM5 )
					case SEESAW_SERCOM0_BASE:
					case SEESAW_SERCOM1_BASE:
					case SEESAW_SERCOM2_BASE:
					case SEESAW_SERCOM3_BASE:
					case SEESAW_SERCOM4_BASE:
					case SEESAW_SERCOM5_BASE:{
						switch(lowByte){
							//TODO: fix for more sercoms
							case SEESAW_SERCOM_STATUS:
							case SEESAW_SERCOM_INTEN:{
								Fifo *fifo = req.getFifo();
								uint8_t dataByte;
								fifo->Read(&dataByte, 1);
								len--;
								
								//read any extra bytes and discard
								me->discard(fifo, len);
								
								Evt *evt = new SercomWriteRegReq(lowByte, dataByte);
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_SERCOM_BAUD:{
								Fifo *fifo = req.getFifo();
								uint8_t baud[4];
								fifo->Read(baud, 4);
								len-=4;
								
								uint32_t combined = ((uint32_t)baud[0] << 24) | ((uint32_t)baud[1] << 16) | ((uint32_t)baud[2] << 8) | (uint32_t)baud[3];
								
								//read any extra bytes and discard
								me->discard(fifo, len);
								
								Evt *evt = new SercomWriteRegReq(lowByte, combined);
								QF::PUBLISH(evt, me);
								break;
							}
							case SEESAW_SERCOM_DATA:{
								//TODO: this should take in number of bytes to write
								Evt *evt = new SercomWriteDataReq(req.getFifo(), req.getLen());
								QF::PUBLISH(evt, me);
								break;
							}
						}
						break;
					}
#endif //SERCOM

#if CONFIG_TIMER
					case SEESAW_TIMER_BASE:{
						switch(lowByte){
							case SEESAW_TIMER_PWM: {
								Fifo *fifo = req.getFifo();
								uint8_t dataBytes[3];
								fifo->Read(dataBytes, 3);
								len -= 3;
								
								me->discard(fifo, len);
								
								Evt *evt = new TimerWritePWM(dataBytes[0], ((uint16_t)dataBytes[1] << 8) | (dataBytes[2]));
								QF::PUBLISH(evt, me);
								
								break;
							}
							case SEESAW_TIMER_FREQ: {
								Fifo *fifo = req.getFifo();
								uint8_t dataBytes[3];
								fifo->Read(dataBytes, 3);
								len -= 3;
								
								me->discard(fifo, len);
								
								Evt *evt = new TimerSetFreq(dataBytes[0], ((uint16_t)dataBytes[1] << 8) | (dataBytes[2]));
								QF::PUBLISH(evt, me);
								
								break;
							}
						}
						break;
					}
#endif //TIMER

#if CONFIG_DAP
					case SEESAW_DAP_BASE:{
						Evt *evt = new DAPRequest(req.getRequesterId(), req.getFifo(), req.getLen());
						QF::PUBLISH(evt, me);
						break;
					}

#endif //DAP

#if CONFIG_EEPROM
					case SEESAW_EEPROM_BASE:{
						Fifo *fifo = req.getFifo();
						uint8_t c[req.getLen()];
						fifo->Read(c, req.getLen());

						eeprom_write(lowByte, c, req.getLen());
						break;
					}
#endif
					
#if CONFIG_NEOPIXEL					
					case SEESAW_NEOPIXEL_BASE: {
						Fifo *fifo = req.getFifo();
						switch(lowByte){
						
							case SEESAW_NEOPIXEL_PIN:{
								uint8_t pin;
								fifo->Read(&pin, 1);
								Evt *evt = new NeopixelSetPinReq(pin);
								QF::PUBLISH(evt, me);
								me->discard(fifo, req.getLen());
								break;
							}
							
							case SEESAW_NEOPIXEL_SPEED:{
								uint8_t speed;
								fifo->Read(&speed, 1);
								Evt *evt = new NeopixelSetSpeedReq(speed);
								QF::PUBLISH(evt, me);
								me->discard(fifo, req.getLen());
								break;
							}
							
							case SEESAW_NEOPIXEL_BUF_LENGTH:{
								uint8_t d[2];
								fifo->Read(d, 2);
								
								Evt *evt = new NeopixelSetBufferLengthReq( ((uint16_t)d[0] << 8) | (uint16_t)d[1]);
								QF::PUBLISH(evt, me);
								
								me->discard(fifo, req.getLen());
								break;
							}
							
							case SEESAW_NEOPIXEL_BUF:{
								uint8_t d[2];
								fifo->Read(d, 2);
								
								Evt *evt = new NeopixelSetBufferReq( ((uint16_t)d[0] << 8) | (uint16_t)d[1], fifo);
								QF::PUBLISH(evt, me);
							}
							break;
						}
						break;
					}
#endif //NEOPIXEL

#if CONFIG_KEYPAD
					case SEESAW_KEYPAD_BASE:{
						Fifo *fifo = req.getFifo();
						uint8_t dataBytes[2];
						fifo->Read(dataBytes, 2);
						len-=2;
						
						Evt *evt = new KeypadWriteRegReq(lowByte, (dataBytes[0] << 8) | dataBytes[1]);
						QF::PUBLISH(evt, me);
						break;
					}
#endif

					default:
						break;
				}
			}
			status = Q_HANDLED();
			
			break;
		}
		
#if CONFIG_INTERRUPT
		case GPIO_INTERRUPT_RECEIVED: {
			Q_ASSERT(Delegate::m_intflag > 0);
			
			Evt *evt = new InterruptSetReq( SEESAW_INTERRUPT_GPIO );
			QF::PUBLISH(evt, me);
			
			status = Q_HANDLED();
			break;
		}
#endif

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

void Delegate::intCallback()
{
	Evt *evt = new Evt(GPIO_INTERRUPT_RECEIVED);
	QF::PUBLISH(evt, 0);
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

void Delegate::break32Bit(uint32_t in, uint8_t *out)
{
	uint8_t b3 = (in >> 24) & 0xFF;
	uint8_t b2 = (in >> 16) & 0xFF;
	uint8_t b1 = (in >> 8) & 0xFF;
	uint8_t b0 = in & 0xFF;
	
	out[0] = b3;
	out[1] = b2;
	out[2] = b1;
	out[3] = b0;
}

extern "C" {
	
	/*
	void EIC_Handler(void)
	{
		QXK_ISR_ENTRY();
		NVIC_DisableIRQ(EIC_IRQn);
		Delegate::intCallback(EIC->INTFLAG.reg);
		EIC->INTFLAG.reg = 0xFFFF;
		QXK_ISR_ENTRY();
	}
	*/
};
