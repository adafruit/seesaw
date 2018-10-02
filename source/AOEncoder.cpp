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
#include "AOEncoder.h"
#include "event.h"
#include "SeesawConfig.h"
#include "bsp_timer.h"
#include "bsp_gpio.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#if CONFIG_ENCODER

#define BIT_IS_SET(x,b) (((x)&(1UL << b)) != 0)
#define BIT_IS_CLEAR(x,b) (((x)&(1UL << b)) == 0)
#define ENCODER_INPUT_MASK ((1UL << CONFIG_ENCODER_A_PIN) | (1UL << CONFIG_ENCODER_B_PIN))

volatile int32_t AOEncoder::m_value;
volatile int32_t AOEncoder::m_delta;
volatile uint8_t AOEncoder::m_enc_prev_pos;
volatile uint8_t AOEncoder::m_enc_flags;
volatile AOEncoder::status AOEncoder::m_status;
AOEncoder::inten AOEncoder::m_inten;

AOEncoder::AOEncoder() :
    QActive((QStateHandler)&AOEncoder::InitialPseudoState), 
    m_id(AO_ENCODER), m_name("Encoder") {}

QState AOEncoder::InitialPseudoState(AOEncoder * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(ENCODER_START_REQ);
    me->subscribe(ENCODER_STOP_REQ);
    me->subscribe(ENCODER_READ_REG_REQ);
    me->subscribe(ENCODER_WRITE_REG_REQ);
      
    return Q_TRAN(&AOEncoder::Root);
}

QState AOEncoder::Root(AOEncoder * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOEncoder::Stopped);
            break;
        }
		case ENCODER_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOEncoder::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOEncoder::Stopped(AOEncoder * const me, QEvt const * const e) {
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
        case ENCODER_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new EncoderStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case ENCODER_START_REQ: {
            LOG_EVENT(e);

            AOEncoder::m_value = 0;
            AOEncoder::m_delta = 0;
            AOEncoder::m_enc_prev_pos = 0;
            AOEncoder::m_enc_flags = 0;
            me->m_status.reg = 0;
            me->m_inten.reg = 0;

            uint32_t mask = ENCODER_INPUT_MASK;
            gpio_dirclr_bulk(PORTA, mask);
			gpio_pullenset_bulk(mask);
			gpio_outset_bulk(PORTA, mask);

            initTimer(CONFIG_ENCODER_TC, CONFIG_ENCODER_FREQ);
            enableTimer(CONFIG_ENCODER_TC);
			
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new EncoderStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOEncoder::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOEncoder::Root);
            break;
        }
    }
    return status;
}

QState AOEncoder::Started(AOEncoder * const me, QEvt const * const e) {
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
		case ENCODER_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new EncoderStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AOEncoder::Stopped);
			break;
		}
        case ENCODER_READ_REG_REQ: {
            LOG_EVENT(e);
            EncoderReadRegReq const &req = static_cast<EncoderReadRegReq const &>(*e);
            Fifo *dest = req.getDest();
			uint8_t reg = req.getReg();

            uint32_t c = 0;
            Evt *evt;

            switch(reg) {
                case SEESAW_ENCODER_POSITION: {
                    c = static_cast<uint32_t>(AOEncoder::m_value);
                    goto data_read;
                    break;
                }

                case SEESAW_ENCODER_DELTA:{
                    c = static_cast<uint32_t>(AOEncoder::m_delta);
                    goto data_read;
                    break;
                }

                default: {
                    goto reg_read_exit;
                    break;
                }
            }
data_read:
            AOEncoder::m_delta = 0;
            if(AOEncoder::m_status.bit.DATA_RDY){
                if(AOEncoder::m_inten.bit.DATA_RDY){
                    Evt *evt = new InterruptClearReq( SEESAW_INTERRUPT_ENCODER_DATA_RDY );
                    QF::PUBLISH(evt, 0);
                }

                AOEncoder::m_status.bit.DATA_RDY = 0;
            }

reg_read_exit:
            uint8_t ret[] = { (uint8_t)(c >> 24), (uint8_t)(c >> 16), (uint8_t)(c >> 8), (uint8_t)(c & 0xFF) };
            dest->Write(ret, 4);
            evt = new DelegateDataReady(req.getRequesterId());
            QF::PUBLISH(evt, me);

            status = Q_HANDLED();
            break;
        }
        case ENCODER_WRITE_REG_REQ: {
            LOG_EVENT(e);
            EncoderWriteRegReq const &req = static_cast<EncoderWriteRegReq const &>(*e);
            int32_t c = req.getValue();
            switch (req.getReg()){
                case SEESAW_ENCODER_POSITION:
                    AOEncoder::m_value = c;
                    break;
                case SEESAW_ENCODER_INTENSET:
                    me->m_inten.reg |= c >> 24;
                    break;
                case SEESAW_ENCODER_INTENCLR:
                    me->m_inten.reg &= ~(c >> 24);
                    break;
                default:
                    break;
            }
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&AOEncoder::Root);
            break;
        }
    }
    return status;
}

void CONFIG_ENCODER_HANDLER( void ) {

    uint32_t in = gpio_read_bulk() & ENCODER_INPUT_MASK;

    int8_t enc_action = 0; // 1 or -1 if moved, sign is direction

    uint8_t enc_cur_pos = 0;
    // read in the encoder state first
    enc_cur_pos |= ((BIT_IS_CLEAR(in, CONFIG_ENCODER_A_PIN)) << 0) | ((BIT_IS_CLEAR(in, CONFIG_ENCODER_B_PIN)) << 1);

    // if any rotation at all
    if (enc_cur_pos != AOEncoder::m_enc_prev_pos)
    {
        if (AOEncoder::m_enc_prev_pos == 0x00)
        {
        // this is the first edge
        if (enc_cur_pos == 0x01) {
            AOEncoder::m_enc_flags |= (1 << 0);
        }
        else if (enc_cur_pos == 0x02) {
            AOEncoder::m_enc_flags |= (1 << 1);
        }
        }

        if (enc_cur_pos == 0x03)
        {
        // this is when the encoder is in the middle of a "step"
        AOEncoder::m_enc_flags |= (1 << 4);
        }
        else if (enc_cur_pos == 0x00)
        {
        // this is the final edge
        if (AOEncoder::m_enc_prev_pos == 0x02) {
            AOEncoder::m_enc_flags |= (1 << 2);
        }
        else if (AOEncoder::m_enc_prev_pos == 0x01) {
            AOEncoder::m_enc_flags |= (1 << 3);
        }

        // check the first and last edge
        // or maybe one edge is missing, if missing then require the middle state
        // this will reject bounces and false movements
        if (BIT_IS_SET(AOEncoder::m_enc_flags, 0) && (BIT_IS_SET(AOEncoder::m_enc_flags, 2) || BIT_IS_SET(AOEncoder::m_enc_flags, 4))) {
            enc_action = 1;
        }
        else if (BIT_IS_SET(AOEncoder::m_enc_flags, 2) && (BIT_IS_SET(AOEncoder::m_enc_flags, 0) || BIT_IS_SET(AOEncoder::m_enc_flags, 4))) {
            enc_action = 1;
        }
        else if (BIT_IS_SET(AOEncoder::m_enc_flags, 1) && (BIT_IS_SET(AOEncoder::m_enc_flags, 3) || BIT_IS_SET(AOEncoder::m_enc_flags, 4))) {
            enc_action = -1;
        }
        else if (BIT_IS_SET(AOEncoder::m_enc_flags, 3) && (BIT_IS_SET(AOEncoder::m_enc_flags, 1) || BIT_IS_SET(AOEncoder::m_enc_flags, 4))) {
            enc_action = -1;
        }

        AOEncoder::m_enc_flags = 0; // reset for next time
        }
    }

    AOEncoder::m_enc_prev_pos = enc_cur_pos;

    if(enc_action != 0){
        AOEncoder::m_value += enc_action;
        AOEncoder::m_delta += enc_action;

        //if interrupts are enabled fire an interrupt
        if(!AOEncoder::m_status.bit.DATA_RDY){
            AOEncoder::m_status.bit.DATA_RDY = 1;

            if(AOEncoder::m_inten.bit.DATA_RDY){
                Evt *evt = new InterruptSetReq( SEESAW_INTERRUPT_ENCODER_DATA_RDY );
                QF::PUBLISH(evt, 0);
            }
        }
    }

    //clear the interrupt
    CONFIG_ENCODER_TC->COUNT16.INTFLAG.bit.MC0 = 1;
}

#endif
