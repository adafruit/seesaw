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
#include "I2CSlave.h"
#include "event.h"
#include "bsp_gpio.h"

#include "bsp_sercom.h"
#include "SeesawConfig.h"

Q_DEFINE_THIS_FILE

using namespace FW;

volatile bool transmissionBegun;

static Fifo *m_inFifo;
static Fifo *m_outFifo;

I2CSlave::I2CSlave( Sercom *sercom) :
    QActive((QStateHandler)&I2CSlave::InitialPseudoState), 
    m_id(I2C_SLAVE), m_name("I2C Slave"), m_sercom(sercom) {}

QState I2CSlave::InitialPseudoState(I2CSlave * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(I2C_SLAVE_START_REQ);
    me->subscribe(I2C_SLAVE_STOP_REQ);
      
	me->subscribe(I2C_SLAVE_REQUEST);
	me->subscribe(I2C_SLAVE_RECEIVE);
	  
    return Q_TRAN(&I2CSlave::Root);
}

QState I2CSlave::Root(I2CSlave * const me, QEvt const * const e) {
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
            status = Q_TRAN(&I2CSlave::Stopped);
            break;
        }
		case I2C_SLAVE_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&I2CSlave::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState I2CSlave::Stopped(I2CSlave * const me, QEvt const * const e) {
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
        case I2C_SLAVE_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new I2CSlaveStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case I2C_SLAVE_START_REQ: {
            LOG_EVENT(e);
			
			initSlaveWIRE( me->m_sercom, CONFIG_I2C_SLAVE_ADDR );
			enableWIRE( me->m_sercom );
			NVIC_ClearPendingIRQ( CONFIG_I2C_SLAVE_IRQn );
			
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SDA, 2);
			pinPeripheral(CONFIG_I2C_SLAVE_PIN_SCL, 2);
			
			transmissionBegun = false;
			
			I2CSlaveStartReq const &req = static_cast<I2CSlaveStartReq const &>(*e);
			m_inFifo = req.getInFifo();
			m_outFifo = req.getOutFifo();
			
			Evt *evt = new I2CSlaveStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&I2CSlave::Started);
            break;
        }
        default: {
            status = Q_SUPER(&I2CSlave::Root);
            break;
        }
    }
    return status;
}

QState I2CSlave::Started(I2CSlave * const me, QEvt const * const e) {
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
		case I2C_SLAVE_REQUEST: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			break;
		}
		case I2C_SLAVE_RECEIVE: {
			LOG_EVENT(e);
			status = Q_HANDLED();
			
			//grab the command bytes
			uint8_t cmd[2];
			m_inFifo->Read(cmd, 2);
			
			//send to the delegate to process
			Evt *evt = new DelegateProcessCommand(me->m_id, cmd[0], cmd[1], 1, m_inFifo);
			QF::PUBLISH(evt, me);
			
			break;
		}
        default: {
            status = Q_SUPER(&I2CSlave::Root);
            break;
        }
    }
    return status;
}

void I2CSlave::RequestCallback(){
	Evt *evt = new Evt(I2C_SLAVE_REQUEST);
	QF::PUBLISH(evt, 0);
}

void I2CSlave::ReceiveCallback(){
	Evt *evt = new Evt(I2C_SLAVE_RECEIVE);
	QF::PUBLISH(evt, 0);
}

extern "C" {
	void CONFIG_I2C_SLAVE_HANDLER(void){
		QXK_ISR_ENTRY();
		if(isStopDetectedWIRE( CONFIG_I2C_SLAVE_SERCOM ) ||
		(isAddressMatch( CONFIG_I2C_SLAVE_SERCOM ) && isRestartDetectedWIRE( CONFIG_I2C_SLAVE_SERCOM ) && !isMasterReadOperationWIRE( CONFIG_I2C_SLAVE_SERCOM ))) //Stop or Restart detected
		{
			prepareAckBitWIRE( CONFIG_I2C_SLAVE_SERCOM );
			prepareCommandBitsWire( CONFIG_I2C_SLAVE_SERCOM, 0x03);
			
			I2CSlave::ReceiveCallback();
		}
		else if(isAddressMatch(CONFIG_I2C_SLAVE_SERCOM))  //Address Match
		{
			prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
			prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);

			if(isMasterReadOperationWIRE(CONFIG_I2C_SLAVE_SERCOM)) //Is a request ?
			{
				//m_outFifo->Reset();

				transmissionBegun = true;
				
				I2CSlave::RequestCallback();
			}
		}
		else if(isDataReadyWIRE(CONFIG_I2C_SLAVE_SERCOM))
		{
			if (isMasterReadOperationWIRE(CONFIG_I2C_SLAVE_SERCOM))
			{
				uint8_t c;
				
				uint8_t count = m_outFifo->Read(&c, 1);

				transmissionBegun = sendDataSlaveWIRE(CONFIG_I2C_SLAVE_SERCOM, (count ? c : 0xff) );
			}
			else { //Received data
				uint8_t c = readDataWIRE(CONFIG_I2C_SLAVE_SERCOM);
				if ( m_inFifo->Write(&c, 1) ){
					prepareAckBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
				}
				else {
					prepareNackBitWIRE(CONFIG_I2C_SLAVE_SERCOM);
				}

				prepareCommandBitsWire(CONFIG_I2C_SLAVE_SERCOM, 0x03);
			}
		}
		QXK_ISR_EXIT();
	}	
};
