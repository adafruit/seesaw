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
#include "AOKeypad.h"
#include "event.h"
#include "SeesawConfig.h"

#include "bsp_gpio.h"
#include "bsp_sercom.h"
#include "bsp_adc.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#define KEYPAD_MAX_ROWS 4
#define KEYPAD_MAX_COLS 4

#define KEYPAD_INPUT_MASK (((uint64_t)CONFIG_KEYPAD_COL0 << CONFIG_KEYPAD_COL0_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_COL1 << CONFIG_KEYPAD_COL1_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_COL2 << CONFIG_KEYPAD_COL2_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_COL3 << CONFIG_KEYPAD_COL3_PIN))

#define KEYPAD_OUTPUT_MASK (((uint64_t)CONFIG_KEYPAD_ROW0 << CONFIG_KEYPAD_ROW0_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_ROW1 << CONFIG_KEYPAD_ROW1_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_ROW2 << CONFIG_KEYPAD_ROW2_PIN) \
                        | ((uint64_t)CONFIG_KEYPAD_ROW3 << CONFIG_KEYPAD_ROW3_PIN))

#define KEYPAD_ACTIVE_ROWS ( (CONFIG_KEYPAD_ROW3 << 3) | (CONFIG_KEYPAD_ROW2 << 2) \
                        | (CONFIG_KEYPAD_ROW1 << 1) | (CONFIG_KEYPAD_ROW0) )
                    
#define KEYPAD_ACTIVE_COLS ( (CONFIG_KEYPAD_COL3 << 3) | (CONFIG_KEYPAD_COL2 << 2) \
                        | (CONFIG_KEYPAD_COL1 << 1) | (CONFIG_KEYPAD_COL0) )

#define KEYPAD_EVENT_TO_ROW(e) ( (e)/8 )
#define KEYPAD_EVENT_TO_COL(e) ( (e)%8 )
#define KEYPAD_EVENT(r, c) ( (r)*8 + (c) )

#define KEYPAD_HIGH (1 << KEYPAD_EDGE_HIGH)
#define KEYPAD_LOW (1 << KEYPAD_EDGE_LOW)
#define KEYPAD_RISING (1 << KEYPAD_EDGE_RISING)
#define KEYPAD_FALLING (1 << KEYPAD_EDGE_FALLING)

static const uint32_t keypad_input_masks[] = {
    ((uint32_t)CONFIG_KEYPAD_COL0 << CONFIG_KEYPAD_COL0_PIN),
    ((uint32_t)CONFIG_KEYPAD_COL1 << CONFIG_KEYPAD_COL1_PIN),
    ((uint32_t)CONFIG_KEYPAD_COL2 << CONFIG_KEYPAD_COL2_PIN),
    ((uint32_t)CONFIG_KEYPAD_COL3 << CONFIG_KEYPAD_COL3_PIN)
};

static const uint32_t keypad_output_masks[] = {
    ((uint32_t)CONFIG_KEYPAD_ROW0 << CONFIG_KEYPAD_ROW0_PIN),
    ((uint32_t)CONFIG_KEYPAD_ROW1 << CONFIG_KEYPAD_ROW1_PIN),
    ((uint32_t)CONFIG_KEYPAD_ROW2 << CONFIG_KEYPAD_ROW2_PIN),
    ((uint32_t)CONFIG_KEYPAD_ROW3 << CONFIG_KEYPAD_ROW3_PIN)
};

AOKeypad::AOKeypad() :
    QActive((QStateHandler)&AOKeypad::InitialPseudoState), 
    m_id(AO_KEYPAD), m_name("Keypad"),m_syncTimer(this, KEYPAD_SYNC) {}

QState AOKeypad::InitialPseudoState(AOKeypad * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(KEYPAD_START_REQ);
    me->subscribe(KEYPAD_STOP_REQ);
    me->subscribe(KEYPAD_SYNC);
    me->subscribe(KEYPAD_WRITE_REG_REQ);
    me->subscribe(KEYPAD_READ_REG_REQ);
      
    return Q_TRAN(&AOKeypad::Root);
}

QState AOKeypad::Root(AOKeypad * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOKeypad::Stopped);
            break;
        }
		case KEYPAD_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOKeypad::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOKeypad::Stopped(AOKeypad * const me, QEvt const * const e) {
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
        case KEYPAD_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new KeypadStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case KEYPAD_START_REQ: {
            LOG_EVENT(e);
			
            //set inputs
            uint32_t mask = (uint32_t)KEYPAD_INPUT_MASK;
			gpio_dirclr_bulk(PORTA, mask);
			gpio_pullenset_bulk(mask);
			gpio_outclr_bulk(PORTA, mask);

            //set outputs
            mask = (uint32_t)KEYPAD_OUTPUT_MASK;
			gpio_dirset_bulk(PORTA, mask);
			gpio_outclr_bulk(PORTA, mask);

            memset(me->m_state, 0, 64); //initialize state array to 0
            me->m_status.reg = 0;
            me->m_inten.reg = 0;

            KeypadStartReq const &r = static_cast<KeypadStartReq const &>(*e);
			me->m_fifo = r.getFifo();
            me->m_fifo->Reset();


			Evt const &req = EVT_CAST(*e);
			Evt *evt = new KeypadStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOKeypad::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOKeypad::Root);
            break;
        }
    }
    return status;
}

QState AOKeypad::Started(AOKeypad * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			me->m_syncTimer.armX(CONFIG_KEYPAD_SYNC_INTERVAL, CONFIG_KEYPAD_SYNC_INTERVAL);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->m_syncTimer.disarm();
            status = Q_HANDLED();
            break;
        }
		case KEYPAD_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new KeypadStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AOKeypad::Stopped);
			break;
		}
        case KEYPAD_SYNC: {
            //LOG_EVENT(e);

            keyState *ks;
            uint32_t in;
            bool val;
            keyEvent keyevent;
            for(int i=0; i<KEYPAD_MAX_ROWS; i++){
                if((1 << i) & KEYPAD_ACTIVE_ROWS){
                    //set the row high
                    gpio_outset_bulk(PORTA, keypad_output_masks[i]);
                    //short delay
                    for(int __tmr = 0; __tmr<100; __tmr++) __asm__ volatile ("NOP;");
                    in = gpio_read_bulk() & KEYPAD_INPUT_MASK; //read everything at once
                    for(int j=0; j<KEYPAD_MAX_COLS; j++){
                        if((1 << j) & KEYPAD_ACTIVE_COLS){
                            val = (in & keypad_input_masks[j]) > 0;
                            keyevent.bit.NUM = KEYPAD_EVENT(i, j);
                            ks = &me->m_state[keyevent.bit.NUM];

                            if(ks->bit.ACTIVE & KEYPAD_HIGH && val){
                                keyevent.bit.EDGE = KEYPAD_EDGE_HIGH;
                                me->m_fifo->Write(&keyevent.reg, 1);
                            }
                            if(ks->bit.ACTIVE & KEYPAD_LOW && !val){
                                keyevent.bit.EDGE = KEYPAD_EDGE_LOW;
                                me->m_fifo->Write(&keyevent.reg, 1);
                            }
                            if(ks->bit.ACTIVE & KEYPAD_RISING && !ks->bit.STATE && val){
                                keyevent.bit.EDGE = KEYPAD_EDGE_RISING;
                                me->m_fifo->Write(&keyevent.reg, 1);
                            }
                            if(ks->bit.ACTIVE & KEYPAD_FALLING && ks->bit.STATE && !val){
                                keyevent.bit.EDGE = KEYPAD_EDGE_FALLING;
                                me->m_fifo->Write(&keyevent.reg, 1);
                            }

                            ks->bit.STATE = val;
                        }
                    }
                    //set the row back low
                    gpio_outclr_bulk(PORTA, keypad_output_masks[i]);
                }
            }

            //create an interrupt if there are events in the FIFO
            if(me->m_fifo->GetUsedCount() > 0){
                me->m_status.bit.DATA_RDY = 1;
                if(me->m_inten.bit.DATA_RDY){
                    //post an interrupt event
                    Evt *evt = new InterruptSetReq( SEESAW_INTERRUPT_KEYPAD_DATA_RDY );
                    QF::PUBLISH(evt, me);
                }
            }
            
            status = Q_HANDLED();
            break;
        }
        case KEYPAD_READ_REG_REQ: {
            LOG_EVENT(e);
            KeypadReadRegReq const &req = static_cast<KeypadReadRegReq const &>(*e);
            Fifo *dest = req.getDest();
			uint8_t reg = req.getReg();

            uint8_t c = 0;
            Evt *evt;

            if(reg == SEESAW_KEYPAD_FIFO){
                //give the requester our pipe
                evt = new DelegateDataReady(req.getRequesterId(), me->m_fifo);
            }
            else{
                switch (reg){
                    case SEESAW_KEYPAD_STATUS:
                        c = me->m_status.reg;
                        break;
                    case SEESAW_KEYPAD_COUNT:
                        c = me->m_fifo->GetUsedCount();
                        break;
                    default:
                        break;
                }

                //return the read register in the default fifo
                evt = new DelegateDataReady(req.getRequesterId());
                dest->Write(&c, 1);
            }

            QF::PUBLISH(evt, me);

            status = Q_HANDLED();
            break;
        }
        case KEYPAD_WRITE_REG_REQ: {
            LOG_EVENT(e);
            KeypadWriteRegReq const &req = static_cast<KeypadWriteRegReq const &>(*e);
            uint32_t c = req.getValue();
            switch (req.getReg()){
                case SEESAW_KEYPAD_INTENSET:
                    me->m_inten.reg |= c;
                    break;
                case SEESAW_KEYPAD_INTENCLR:
                    me->m_inten.reg &= ~(c);
                    break;
                case SEESAW_KEYPAD_EVENT:
                    //turn an event on or off
                    keyState *ks;
                    ks = &me->m_state[(c >> 8) & 0xFF];

                    if(c & 0x01) //activate the selected edges
                        ks->bit.ACTIVE |= (c >> 1);
                    else //deactivate the selected edges
                        ks->bit.ACTIVE &= (c >> 1);

                    break;
                default:
                    break;
            }
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&AOKeypad::Root);
            break;
        }
    }
    return status;
}