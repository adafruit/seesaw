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


#ifndef AO_SERCOM_H
#define AO_SERCOM_H

#include "qpcpp.h"
#include "qp_extras.h"

#include "hsm_id.h"

#include "sam.h"

using namespace QP;
using namespace FW;

class AOSERCOM : public QActive {
public:
    AOSERCOM( Sercom *sercom, uint8_t id, uint8_t offset);
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }
	
	static void RxCallback();

protected:
    static QState InitialPseudoState(AOSERCOM * const me, QEvt const * const e);
    static QState Root(AOSERCOM * const me, QEvt const * const e);
    static QState Stopped(AOSERCOM * const me, QEvt const * const e);
    static QState Started(AOSERCOM * const me, QEvt const * const e);
	
	static QState UART(AOSERCOM * const me, QEvt const * const e);
	static QState SPI(AOSERCOM * const me, QEvt const * const e);

    enum {
        EVT_QUEUE_COUNT = 16,
    };
    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    uint8_t m_id;
	uint16_t m_nextSequence;
    char const * m_name;
	
	Sercom *m_sercom;
	uint8_t m_offset;
	
	uint32_t m_baud;
	
	// The status register
    struct status {
           
        /* 0: no error
        *  1: error has occurred
        */ 
        uint8_t ERROR: 1;

        /* 0: no data is ready
        *  1: data is ready
        */ 
        uint8_t DATA_RDY: 1;

        uint8_t get(){
			return (DATA_RDY << 1) | ERROR;
        }
		void set(uint8_t data){
			ERROR = data & 0x01;
			DATA_RDY = (data & 0x02) >> 1;
		}
    };
    status m_status;
	
	struct inten {
		uint8_t DATA_RDY: 1;
		
		uint8_t get(){
			return DATA_RDY;
		}
		void set(uint8_t data){
			DATA_RDY = data & 0x01;
		}
	};
	inten m_inten;
	
	struct intenclr {
		uint8_t DATA_RDY: 1;
		
		void set(uint8_t data){
			DATA_RDY = data & 0x01;
		}
	};
	intenclr m_intenclr;
};


#endif // AO_SERCOM_H


