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
#include "AOInterrupt.h"
#include "event.h"

#include "bsp_gpio.h"
#include "PinMap.h"
#include "SeesawConfig.h"

Q_DEFINE_THIS_FILE

using namespace FW;

AOInterrupt::AOInterrupt() :
    QActive((QStateHandler)&AOInterrupt::InitialPseudoState), 
    m_id(AO_INTERRUPT), m_name("Interrupt"), m_pin(g_APinDescription[CONFIG_INTERRUPT_PIN]) {}

QState AOInterrupt::InitialPseudoState(AOInterrupt * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(INTERRUPT_START_REQ);
    me->subscribe(INTERRUPT_STOP_REQ);
	
	me->subscribe(INTERRUPT_SET_REQ);
	me->subscribe(INTERRUPT_CLEAR_REQ);
      
    return Q_TRAN(&AOInterrupt::Root);
}

QState AOInterrupt::Root(AOInterrupt * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOInterrupt::Stopped);
            break;
        }
		case INTERRUPT_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOInterrupt::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOInterrupt::Stopped(AOInterrupt * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			me->m_intflag = 0;
			
			gpio_init(me->m_pin.ulPort, me->m_pin.ulPin, 1); //set as output
			gpio_write(me->m_pin.ulPort, me->m_pin.ulPin, 1); //write high
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case INTERRUPT_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new InterruptStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case INTERRUPT_START_REQ: {
            LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new InterruptStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(&AOInterrupt::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOInterrupt::Root);
            break;
        }
    }
    return status;
}

QState AOInterrupt::Started(AOInterrupt * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
			gpio_init(me->m_pin.ulPort, me->m_pin.ulPin, 1); //set as output
			gpio_write(me->m_pin.ulPort, me->m_pin.ulPin, 1); //write high
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
			status = Q_TRAN(&AOInterrupt::Unasserted);
			break;
		}
		case INTERRUPT_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new InterruptStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(&AOInterrupt::Stopped);
			break;
		}
		case INTERRUPT_SET_REQ: {
			
			InterruptSetReq const &req = static_cast<InterruptSetReq const &>(*e);
			
			me->m_intflag |= req.getId();
			status = Q_TRAN(&AOInterrupt::Asserted);
			
			break;
		}
		case INTERRUPT_CLEAR_REQ: {
			
			InterruptClearReq const &req = static_cast<InterruptClearReq const &>(*e);
			
			me->m_intflag &= ~req.getId();
			
			if(!me->m_intflag)
				status = Q_TRAN(&AOInterrupt::Unasserted);
				
			else status = Q_HANDLED();
			
			break;
		}
        default: {
            status = Q_SUPER(&AOInterrupt::Root);
            break;
        }
    }
    return status;
}

QState AOInterrupt::Unasserted(AOInterrupt * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			
			gpio_write(me->m_pin.ulPort, me->m_pin.ulPin, 1); //write high
			
			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			break;
		}
		default: {
			status = Q_SUPER(&AOInterrupt::Started);
			break;
		}
	}
	return status;
}

QState AOInterrupt::Asserted(AOInterrupt * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			
			gpio_write(me->m_pin.ulPort, me->m_pin.ulPin, 0); //write low
			
			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			break;
		}
		default: {
			status = Q_SUPER(&AOInterrupt::Started);
			break;
		}
	}
	return status;
}
