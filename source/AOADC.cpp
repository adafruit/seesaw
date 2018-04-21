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
#include "AOADC.h"
#include "event.h"

#include "bsp_adc.h"
#include "bsp_gpio.h"

#include "SeesawConfig.h"
#include "RegisterMap.h"

Q_DEFINE_THIS_FILE

using namespace FW;

AOADC::AOADC() :
    QActive((QStateHandler)&AOADC::InitialPseudoState), 
    m_id(AO_ADC), m_name("ADC") {}

QState AOADC::InitialPseudoState(AOADC * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(ADC_START_REQ);
    me->subscribe(ADC_STOP_REQ);
	
	me->subscribe(ADC_WRITE_WINMON_REQ);
	me->subscribe(ADC_WRITE_REG_REQ);
	me->subscribe(ADC_READ_REG_REQ);
      
    return Q_TRAN(&AOADC::Root);
}

QState AOADC::Root(AOADC * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOADC::Stopped);
            break;
        }
		case ADC_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOADC::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOADC::Stopped(AOADC * const me, QEvt const * const e) {
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
        case ADC_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new ADCStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case ADC_START_REQ: {
            LOG_EVENT(e);

//DM: TODO: this is used for freerunning adc
#if CONFIG_ADC_INPUT_3
#define ADC_NUM_INPUT 4
#elif CONFIG_ADC_INPUT_2
#define ADC_NUM_INPUT 3
#elif CONFIG_ADC_INPUT_1
#define ADC_NUM_INPUT 2
#elif CONFIG_ADC_INPUT_0
#define ADC_NUM_INPUT 1
#else
			Q_ASSERT(0); //at least one ADC input must be defined
#endif

			adc_init();
			
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new ADCStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(&AOADC::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOADC::Root);
            break;
        }
    }
    return status;
}

QState AOADC::Started(AOADC * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
			status = Q_TRAN(&AOADC::Normal);
			break;
		}
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		case ADC_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new ADCStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AOADC::Stopped);
			break;
		}
		case ADC_READ_REG_REQ: {
			LOG_EVENT(e);
			ADCReadRegReq const &req = static_cast<ADCReadRegReq const &>(*e);
			Fifo *dest = req.getDest();
			uint8_t reg = req.getReg();
			
			//there should be nothing in the destination pipe
			Q_ASSERT(!dest->GetUsedCount());

#if CONFIG_ADC
			uint16_t valueRead = 0;
			switch(reg){
#if CONFIG_ADC_INPUT_0
				case SEESAW_ADC_CHANNEL_0: {
                    pinPeripheral(CONFIG_ADC_INPUT_0_PIN, 1);
                    valueRead = adc_read(CONFIG_ADC_INPUT_0_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_1
				case SEESAW_ADC_CHANNEL_1:{
				    pinPeripheral(CONFIG_ADC_INPUT_1_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_1_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_2
				case SEESAW_ADC_CHANNEL_2:{
				    pinPeripheral(CONFIG_ADC_INPUT_2_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_2_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_3
				case SEESAW_ADC_CHANNEL_3:{
				    pinPeripheral(CONFIG_ADC_INPUT_3_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_3_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_4
				case SEESAW_ADC_CHANNEL_4:{
				    pinPeripheral(CONFIG_ADC_INPUT_4_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_4_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_5
				case SEESAW_ADC_CHANNEL_5:{
				    pinPeripheral(CONFIG_ADC_INPUT_5_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_5_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_6
				case SEESAW_ADC_CHANNEL_6:{
				    pinPeripheral(CONFIG_ADC_INPUT_6_PIN, 1);
				    valueRead = adc_read(CONFIG_ADC_INPUT_6_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
#if CONFIG_ADC_INPUT_7
				case SEESAW_ADC_CHANNEL_7:{
				    pinPeripheral(CONFIG_ADC_INPUT_7_PIN, 1);
                    valueRead = adc_read(CONFIG_ADC_INPUT_7_CHANNEL);
                    goto seesaw_adc_read;
				}
#endif
				default:
seesaw_adc_read:
                    uint8_t ret[] = { (uint8_t)(valueRead >> 8), (uint8_t)(valueRead & 0xFF) };
                    dest->Write(ret, 2);
					break;
			}
			
			Evt *evt = new DelegateDataReady(req.getRequesterId());
			QF::PUBLISH(evt, me);
#endif
			status = Q_HANDLED();
			break;
		}
		case ADC_WRITE_REG_REQ: {
			LOG_EVENT(e);
			ADCWriteRegReq const &req = static_cast<ADCWriteRegReq const &>(*e);
			uint8_t reg = req.getReg();
			
			switch (reg){
				case SEESAW_ADC_INTEN:{
					me->m_inten.set(req.getValue());
					ADC->INTENSET.bit.WINMON = me->m_inten.WINMON;
					break;
				}
				case SEESAW_ADC_INTENCLR:{
					me->m_inten.clr(req.getValue());
					ADC->INTENSET.bit.WINMON = me->m_inten.WINMON;
					break;
				}
				case SEESAW_ADC_WINMODE:{
					ADC->WINCTRL.reg = req.getValue();
					break;
				}
				default:
				//TODO: lets handle this error better
				Q_ASSERT(0);
				break;
			}
			
			status = Q_HANDLED();
			break;
		}
		case ADC_WRITE_WINMON_REQ: {
			LOG_EVENT(e);
			ADCWriteWinmonThresh const &req = static_cast<ADCWriteWinmonThresh const &>(*e);
			ADC->WINLT.reg = req.getLower();
			ADC->WINUT.reg = req.getUpper();
			status = Q_HANDLED();
			break;
		}
        default: {
            status = Q_SUPER(&AOADC::Root);
            break;
        }
    }
    return status;
}

QState AOADC::Normal(AOADC * const me, QEvt const * const e) {
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
			status = Q_SUPER(&AOADC::Started);
			break;
		}
	}
	return status;
}

QState AOADC::Freeruning(AOADC * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			adc_set_inputscan(ADC_NUM_INPUT - 1);
			adc_set_freerunning(true);
			adc_trigger();
			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			adc_set_inputscan(0);
			adc_set_freerunning(false);
			status = Q_HANDLED();
			break;
		}
		//TODO: not rly sure how this should work just yet
		case ADC_READ_REG_REQ: {
			ADCReadRegReq const &req = static_cast<ADCReadRegReq const &>(*e);
			Fifo *dest = req.getDest();
			uint8_t reg = req.getReg();
			
			//there should be nothing in the destination pipe
			Q_ASSERT(!dest->GetUsedCount());

			if(reg >= SEESAW_ADC_CHANNEL_0){
				
				ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
				//read the data
				while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
				uint16_t valueRead = ADC->RESULT.reg;
				uint8_t ret[] = { (uint8_t)(valueRead >> 8), (uint8_t)(valueRead & 0xFF) };
				dest->Write(ret, 2);
				
				Evt *evt = new DelegateDataReady(req.getRequesterId());
				QF::PUBLISH(evt, me);
				
				status = Q_HANDLED();
				
				break;
			}
		}
		default: {
			status = Q_SUPER(&AOADC::Started);
			break;
		}
	}
	return status;
}
