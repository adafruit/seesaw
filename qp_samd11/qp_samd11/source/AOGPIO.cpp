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
#include "AOGPIO.h"
#include "event.h"

#include "bsp_gpio.h"

Q_DEFINE_THIS_FILE

using namespace FW;

AOGPIO::AOGPIO() :
    QActive((QStateHandler)&AOGPIO::InitialPseudoState), 
    m_id(AO_GPIO), m_name("GPIO") {}

QState AOGPIO::InitialPseudoState(AOGPIO * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(GPIO_START_REQ);
    me->subscribe(GPIO_STOP_REQ);
	
	me->subscribe(GPIO_WRITE);
      
    return Q_TRAN(&AOGPIO::Root);
}

QState AOGPIO::Root(AOGPIO * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOGPIO::Stopped);
            break;
        }
		case GPIO_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOGPIO::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOGPIO::Stopped(AOGPIO * const me, QEvt const * const e) {
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
        case GPIO_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new GPIOStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case GPIO_START_REQ: {
            LOG_EVENT(e);
			
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new GPIOStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOGPIO::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOGPIO::Root);
            break;
        }
    }
    return status;
}

QState AOGPIO::Started(AOGPIO * const me, QEvt const * const e) {
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
		
		case GPIO_WRITE: {
			
			GPIOWrite const &req = static_cast<GPIOWrite const &>(*e);
			
			gpio_init(0, 16, 1);
			//gpio_write(0, 16, req.getValue());
			gpio_toggle(0, 16);
			
			status = Q_HANDLED();
			break;
		}
		
        default: {
            status = Q_SUPER(&AOGPIO::Root);
            break;
        }
    }
    return status;
}
