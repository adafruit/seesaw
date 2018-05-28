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
#include "AOTouch.h"
#include "event.h"

#include "adafruit_ptc.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#define _GET_Y_LINE(x,y)    if ((x) < 32) { \
                                if (((x) >= 2) && ((x) <= 7)) { \
                                      y = ((x) - 2); \
                                } \
                            } \
                            else { \
                                if (((x - 32) >= 0) && ((x - 32) <= 9)) { \
                                    y = ((x - 32) + 6); \
                                } \
                            } \

AOTouch::AOTouch() :
    QActive((QStateHandler)&AOTouch::InitialPseudoState),
    m_id(AO_TOUCH), m_name("Touch") {

#if CONFIG_TOUCH
    oversample_t f = OVERSAMPLE_4;
    series_resistor_t r = RESISTOR_50K;
    freq_mode_t fh = FREQ_MODE_NONE;

#if CONFIG_TOUCH0
    adafruit_ptc_get_config_default(&config0);
    config0.pin = CONFIG_TOUCH0_PIN;

    _GET_Y_LINE(config0.pin,config0.yline)
    config0.oversample = f;
    config0.seriesres = r;
    config0.freqhop = fh;
#endif

#if CONFIG_TOUCH1
    adafruit_ptc_get_config_default(&config1);
    config1.pin = CONFIG_TOUCH1_PIN;

    _GET_Y_LINE(config1.pin,config1.yline)
    config1.oversample = f;
    config1.seriesres = r;
    config1.freqhop = fh;
#endif

#if CONFIG_TOUCH2
    adafruit_ptc_get_config_default(&config2);
    config2.pin = CONFIG_TOUCH2_PIN;

    _GET_Y_LINE(config2.pin,config2.yline)
    config2.oversample = f;
    config2.seriesres = r;
    config2.freqhop = fh;
#endif

#if CONFIG_TOUCH3
    adafruit_ptc_get_config_default(&config3);
    config3.pin = CONFIG_TOUCH3_PIN;

    _GET_Y_LINE(config3.pin,config3.yline)
    config3.oversample = f;
    config3.seriesres = r;
    config3.freqhop = fh;
#endif

#endif
}

QState AOTouch::InitialPseudoState(AOTouch * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(TOUCH_START_REQ);
    me->subscribe(TOUCH_STOP_REQ);
    me->subscribe(TOUCH_READ_REG_REQ);
      
    return Q_TRAN(&AOTouch::Root);
}

QState AOTouch::Root(AOTouch * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOTouch::Stopped);
            break;
        }
		case TOUCH_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOTouch::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOTouch::Stopped(AOTouch * const me, QEvt const * const e) {
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
        case TOUCH_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new TouchStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case TOUCH_START_REQ: {
            LOG_EVENT(e);
			
            init_clocks_ptc();

#if CONFIG_TOUCH0
            adafruit_ptc_init((Ptc*)PTC, &me->config0);
#endif

#if CONFIG_TOUCH1
            adafruit_ptc_init((Ptc*)PTC, &me->config1);
#endif

#if CONFIG_TOUCH2
            adafruit_ptc_init((Ptc*)PTC, &me->config2);
#endif

#if CONFIG_TOUCH3
            adafruit_ptc_init((Ptc*)PTC, &me->config3);
#endif

			Evt const &req = EVT_CAST(*e);
			Evt *evt = new TouchStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOTouch::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOTouch::Root);
            break;
        }
    }
    return status;
}

QState AOTouch::Started(AOTouch * const me, QEvt const * const e) {
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
		case TOUCH_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new TouchStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AOTouch::Stopped);
			break;
		}
		case TOUCH_READ_REG_REQ: {
		    LOG_EVENT(e);
            TouchReadRegReq const &req = static_cast<TouchReadRegReq const &>(*e);

#if CONFIG_TOUCH
            Fifo *dest = req.getDest();
            uint8_t reg = req.getReg();

            //there should be nothing in the destination pipe
            Q_ASSERT(!dest->GetUsedCount());

            uint16_t valueRead = 0;
            switch(reg){
#if CONFIG_TOUCH0
            case SEESAW_TOUCH_CHANNEL_0: {
                valueRead = touch_measure(&me->config0);
                goto seesaw_touch_read;
            }
#endif
#if CONFIG_TOUCH1
            case SEESAW_TOUCH_CHANNEL_1: {
                valueRead = touch_measure(&me->config1);
                goto seesaw_touch_read;
            }
#endif
#if CONFIG_TOUCH2
            case SEESAW_TOUCH_CHANNEL_2: {
                valueRead = touch_measure(&me->config2);
                goto seesaw_touch_read;
            }
#endif
#if CONFIG_TOUCH3
            case SEESAW_TOUCH_CHANNEL_3: {
                valueRead = touch_measure(&me->config3);
                goto seesaw_touch_read;
            }
#endif
            default:
seesaw_touch_read:
                uint8_t ret[] = { (uint8_t)(valueRead >> 8), (uint8_t)(valueRead & 0xFF) };
                dest->Write(ret, 2);
                break;
            }
#endif

            Evt *evt = new DelegateDataReady(req.getRequesterId());
            QF::PUBLISH(evt, me);

            status = Q_HANDLED();
            break;
		}
        default: {
            status = Q_SUPER(&AOTouch::Root);
            break;
        }
    }
    return status;
}
