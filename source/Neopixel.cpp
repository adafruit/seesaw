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
#include "Neopixel.h"
#include "event.h"
#include "SeesawConfig.h"
#include "bsp_gpio.h"
#include "bsp_neopix.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#define SEESAW_NEOPIXEL_400KHZ 0
#define SEESAW_NEOPIXEL_800KHZ 1

static uint8_t PixelData[CONFIG_NEOPIXEL_BUF_MAX];

Neopixel::Neopixel() :
    QActive((QStateHandler)&Neopixel::InitialPseudoState), 
    m_id(AO_NEOPIXEL), m_name("Neopix") {}

QState Neopixel::InitialPseudoState(Neopixel * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(NEOPIXEL_START_REQ);
    me->subscribe(NEOPIXEL_STOP_REQ);
	me->subscribe(NEOPIXEL_SET_PIN_REQ);
	me->subscribe(NEOPIXEL_SET_SPEED_REQ);
	me->subscribe(NEOPIXEL_SET_BUFFER_LEN_REQ);
	me->subscribe(NEOPIXEL_SET_BUFFER_REQ);
	me->subscribe(NEOPIXEL_SHOW_REQ);
      
    return Q_TRAN(&Neopixel::Root);
}

QState Neopixel::Root(Neopixel * const me, QEvt const * const e) {
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
            status = Q_TRAN(&Neopixel::Stopped);
            break;
        }
		case NEOPIXEL_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&Neopixel::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState Neopixel::Stopped(Neopixel * const me, QEvt const * const e) {
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
        case NEOPIXEL_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new NeopixelStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case NEOPIXEL_START_REQ: {
            LOG_EVENT(e);
			me->m_speed = SEESAW_NEOPIXEL_800KHZ;
			memset(PixelData, 0, CONFIG_NEOPIXEL_BUF_MAX);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new NeopixelStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&Neopixel::Started);
            break;
        }
        default: {
            status = Q_SUPER(&Neopixel::Root);
            break;
        }
    }
    return status;
}

QState Neopixel::Started(Neopixel * const me, QEvt const * const e) {
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
		case NEOPIXEL_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new NeopixelStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(Neopixel::Stopped);
			break;
		}
		case NEOPIXEL_SET_PIN_REQ: {
			NeopixelSetPinReq const &req = static_cast<NeopixelSetPinReq const &>(*e);
			gpio_init(PORTA, req.getPin(), 1);
			me->m_pin = req.getPin();
			
			status = Q_HANDLED();
			break;
		}
		case NEOPIXEL_SET_SPEED_REQ: {
			NeopixelSetSpeedReq const &req = static_cast<NeopixelSetSpeedReq const &>(*e);
			
			me->m_speed = req.getSpeed();
			status = Q_HANDLED();
			
			break;
		}
		case NEOPIXEL_SET_BUFFER_LEN_REQ: {
			NeopixelSetBufferLengthReq const &req = static_cast<NeopixelSetBufferLengthReq const &>(*e);
			uint16_t len = req.getLen();
			
			Q_ASSERT(len <= CONFIG_NEOPIXEL_BUF_MAX);
			
			me->m_pixelDataSize = len;
			status = Q_HANDLED();
			break;
		}
		case NEOPIXEL_SET_BUFFER_REQ: {
			
			NeopixelSetBufferReq const &req = static_cast<NeopixelSetBufferReq const &>(*e);
			uint16_t start = req.getAddr();
			Fifo *fifo = req.getSource();
			uint16_t len = fifo->GetUsedCount();
			len = (len > CONFIG_NEOPIXEL_BUF_MAX - start ? CONFIG_NEOPIXEL_BUF_MAX - start : len);
			
			fifo->Read(PixelData + start, len);
			
			//discard any extra data
			fifo->Reset();
			
			status = Q_HANDLED();
			break;
		}
		case NEOPIXEL_SHOW_REQ: {
			QF_CRIT_STAT_TYPE crit;
			QF_CRIT_ENTRY(crit);
			switch(me->m_speed){
				case SEESAW_NEOPIXEL_800KHZ:
					neopix_show_800k(me->m_pin, PixelData, me->m_pixelDataSize);
					break;
				case SEESAW_NEOPIXEL_400KHZ:
					neopix_show_400k(me->m_pin, PixelData, me->m_pixelDataSize);
					break;
				default:
					break;
			}
			QF_CRIT_EXIT(crit);
		}
        default: {
            status = Q_SUPER(&Neopixel::Root);
            break;
        }
    }
    return status;
}
