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
#include "AOTimer.h"
#include "event.h"

#include "bsp_timer.h"
#include "bsp_gpio.h"
#include "SeesawConfig.h"

Q_DEFINE_THIS_FILE

using namespace FW;

AOTimer::AOTimer() :
    QActive((QStateHandler)&AOTimer::InitialPseudoState), 
    m_id(AO_TIMER), m_name("TIMER") {}

QState AOTimer::InitialPseudoState(AOTimer * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(TIMER_START_REQ);
    me->subscribe(TIMER_STOP_REQ);
    
	me->subscribe(TIMER_WRITE_PWM);
	me->subscribe(TIMER_SET_FREQ);
	
    return Q_TRAN(&AOTimer::Root);
}

QState AOTimer::Root(AOTimer * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOTimer::Stopped);
            break;
        }
		case TIMER_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOTimer::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOTimer::Stopped(AOTimer * const me, QEvt const * const e) {
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
        case TIMER_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new TimerStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case TIMER_START_REQ: {
            LOG_EVENT(e);
			
#if CONFIG_TIMER_PWM_OUT0
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT0_IS_TCC
            if(CONFIG_TIMER_PWM_OUT0_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT0_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT0_PIN, 5);

			initTimerPWM( CONFIG_TIMER_PWM_OUT0_TCC );
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT0_PIN, 4);
			initTimerPWM( CONFIG_TIMER_PWM_OUT0_TC );
#endif //NOT_ON_TC
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT0_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT0_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT1
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT1_IS_TCC
            if(CONFIG_TIMER_PWM_OUT1_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT1_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT1_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT1_TCC );
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT1_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT1_TC );
#endif //NOT_ON_TC
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT1_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT1_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT2
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT2_IS_TCC
            if(CONFIG_TIMER_PWM_OUT2_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT2_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT2_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT2_TCC );
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT2_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT2_TC );
#endif //NOT_ON_TC
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT2_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT2_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT3
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT3_IS_TCC
            if(CONFIG_TIMER_PWM_OUT3_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT3_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT3_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT3_TCC );
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT3_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT3_TC );
#endif //NOT_ON_TC
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT3_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT3_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT4
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT4_IS_TCC
            if(CONFIG_TIMER_PWM_OUT4_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT4_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT4_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT4_TCC );
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT4_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT4_TC );
#endif //NOT_ON_TC
#else 
            pinPeripheral(CONFIG_TIMER_PWM_OUT4_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT4_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT5
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT5_IS_TCC
            if(CONFIG_TIMER_PWM_OUT5_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT5_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT5_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT5_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT5_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT5_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT5_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT5_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT6
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT6_IS_TCC
            if(CONFIG_TIMER_PWM_OUT6_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT6_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT6_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT6_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT6_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT6_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT6_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT6_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT7
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT7_IS_TCC
            if(CONFIG_TIMER_PWM_OUT7_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT7_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT7_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT7_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT7_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT7_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT7_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT7_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT8
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT8_IS_TCC
            if(CONFIG_TIMER_PWM_OUT8_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT8_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT8_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT8_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT8_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT8_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT8_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT8_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT9
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT9_IS_TCC
            if(CONFIG_TIMER_PWM_OUT9_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT9_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT9_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT9_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT9_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT9_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT9_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT9_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT10
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT10_IS_TCC
            if(CONFIG_TIMER_PWM_OUT10_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT10_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT10_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT10_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT10_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT10_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT10_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT10_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

#if CONFIG_TIMER_PWM_OUT11
#ifdef USE_TCC_TIMERS
#ifdef CONFIG_TIMER_PWM_OUT11_IS_TCC
            if(CONFIG_TIMER_PWM_OUT11_TCC == TCC2)
                pinPeripheral(CONFIG_TIMER_PWM_OUT11_PIN, 4);
            else
                pinPeripheral(CONFIG_TIMER_PWM_OUT11_PIN, 5);

            initTimerPWM( CONFIG_TIMER_PWM_OUT11_TCC );
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT11_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT11_TC );
#endif //NOT_ON_TC
#else
            pinPeripheral(CONFIG_TIMER_PWM_OUT11_PIN, 4);
            initTimerPWM( CONFIG_TIMER_PWM_OUT11_TC );
#endif //USE_TCC_TIMERS
#endif //CONFIG_TIMER_PWM_OUTX

			Evt const &req = EVT_CAST(*e);
			Evt *evt = new TimerStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(&AOTimer::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOTimer::Root);
            break;
        }
    }
    return status;
}

QState AOTimer::Started(AOTimer * const me, QEvt const * const e) {
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
		case TIMER_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new TimerStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AOTimer::Stopped);
			break;
		}
		case TIMER_WRITE_PWM: {
			
			TimerWritePWM const &req = static_cast<TimerWritePWM const &>(*e);
			PWMWrite(req.getPwm(), req.getValue());
			
			status = Q_HANDLED();
			break;
		}
		case TIMER_SET_FREQ: {
			
			TimerSetFreq const &req = static_cast<TimerSetFreq const &>(*e);
			setFreq(req.getPwm(), req.getFreq());
			
			status = Q_HANDLED();
			break;
		}
        default: {
            status = Q_SUPER(&AOTimer::Root);
            break;
        }
    }
    return status;
}
