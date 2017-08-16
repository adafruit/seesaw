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
#include "AODAP.h"
#include "event.h"
#include "dap.h"
#include "dap_config.h"

Q_DEFINE_THIS_FILE

using namespace FW;

static Fifo *m_rxFifo;

static uint8_t inbuf[DAP_CONFIG_PACKET_SIZE];
static uint8_t outbuf[DAP_CONFIG_PACKET_SIZE];

AODAP::AODAP() :
    QActive((QStateHandler)&AODAP::InitialPseudoState), 
    m_id(AO_DAP), m_name("DAP") {}

QState AODAP::InitialPseudoState(AODAP * const me, QEvt const * const e) {
    (void)e;

    me->subscribe(DAP_START_REQ);
    me->subscribe(DAP_STOP_REQ);
	me->subscribe(DAP_REQUEST);
	me->subscribe(DAP_READ);
      
    return Q_TRAN(&AODAP::Root);
}

QState AODAP::Root(AODAP * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AODAP::Stopped);
            break;
        }
		case DAP_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AODAP::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AODAP::Stopped(AODAP * const me, QEvt const * const e) {
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
        case DAP_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new DAPStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case DAP_START_REQ: {
            LOG_EVENT(e);
			DAPStartReq const &req = static_cast<DAPStartReq const &>(*e);
			m_rxFifo = req.getRxFifo();
			dap_init();
			Evt *evt = new DAPStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AODAP::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AODAP::Root);
            break;
        }
    }
    return status;
}

QState AODAP::Started(AODAP * const me, QEvt const * const e) {
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
		case DAP_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new DAPStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			status = Q_TRAN(AODAP::Stopped);
			break;
		}
		case DAP_REQUEST:{
			DAPRequest const &req = static_cast<DAPRequest const &>(*e);
			Fifo *source = req.getSource();
			uint8_t len = req.getLen();
			source->Read(inbuf, len);
			
			QF_CRIT_STAT_TYPE crit;
			QF_CRIT_ENTRY(crit);
			dap_process_request(inbuf, outbuf);
			QF_CRIT_EXIT(crit);
			
			//copy the outbuffer to the FIFO
			//TODO: should we do this?
			m_rxFifo->Reset();
			m_rxFifo->Write(outbuf, DAP_CONFIG_PACKET_SIZE);
			
			status = Q_HANDLED();
			break;
		}
		case DAP_READ:{
			DAPRead const &req = static_cast<DAPRead const &>(*e);
			
			Evt *evt = new DelegateDataReady(req.getRequesterId(), m_rxFifo);
			QF::PUBLISH(evt, me);
			break;
		}
        default: {
            status = Q_SUPER(&AODAP::Root);
            break;
        }
    }
    return status;
}
