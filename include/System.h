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


#ifndef SYSTEM_H
#define SYSTEM_H

#include "qpcpp.h"
#include "qp_extras.h"

#include "hsm_id.h"

#include "SeesawConfig.h"

using namespace QP;
using namespace FW;

class System : public QActive {
public:
    System();
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }

protected:
    static QState InitialPseudoState(System * const me, QEvt const * const e);
    static QState Root(System * const me, QEvt const * const e);
    static QState Stopped(System * const me, QEvt const * const e);
	static QState Stopping(System * const me, QEvt const * const e);
	static QState Starting(System * const me, QEvt const * const e);
    static QState Started(System * const me, QEvt const * const e);
    static QState Idle(System * const me, QEvt const * const e);
    static QState Conflicted(System * const me, QEvt const * const e);
	
	void HandleCfm(ErrorEvt const &e, uint8_t expectedCnt);

    enum {
        EVT_QUEUE_COUNT = 16,
        //DEFER_QUEUE_COUNT = 4
    };
    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    //QEvt const *m_deferQueueStor[DEFER_QUEUE_COUNT];
    //QEQueue m_deferQueue;
    uint8_t m_id;
	uint16_t m_nextSequence;
	uint16_t m_savedInSeq;
	uint8_t m_cfmCount;
    char const * m_name;
	
#if CONFIG_POWER_SENSE
    QTimeEvt m_powerSenseTimer;
    QTimeEvt m_powerSenseBlinkTimer;
    uint8_t m_powerSenseLEDState;
#endif

#if CONFIG_I2C_SLAVE
	enum {
		I2C_SLAVE_OUT_FIFO_ORDER = 5,
		I2C_SLAVE_IN_FIFO_ORDER = 5,
	};
	
	uint8_t m_I2CSlaveOutFifoStor[1 << I2C_SLAVE_OUT_FIFO_ORDER];
	uint8_t m_I2CSlaveInFifoStor[1 << I2C_SLAVE_IN_FIFO_ORDER];
	Fifo m_I2CSlaveOutFifo;
	Fifo m_I2CSlaveInFifo;
#endif

#if CONFIG_SPI_SLAVE
	enum {
		SPI_SLAVE_OUT_FIFO_ORDER = 5,
		SPI_SLAVE_IN_FIFO_ORDER = 5,
	};

	uint8_t m_SPISlaveOutFifoStor[1 << SPI_SLAVE_OUT_FIFO_ORDER];
	uint8_t m_SPISlaveInFifoStor[1 << SPI_SLAVE_IN_FIFO_ORDER];
	Fifo m_SPISlaveOutFifo;
	Fifo m_SPISlaveInFifo;
#endif

#if CONFIG_USB
	enum {
		USB_OUT_FIFO_ORDER = 5,
		USB_IN_FIFO_ORDER = 5,
	};

	uint8_t m_USBOutFifoStor[1 << USB_OUT_FIFO_ORDER];
	uint8_t m_USBInFifoStor[1 << USB_IN_FIFO_ORDER];
	Fifo m_USBOutFifo;
	Fifo m_USBInFifo;
#endif

	enum {
		SERCOM_FIFO_ORDER = 5,
	};
	
	enum {
		DAP_FIFO_ORDER = 7,
	};

	enum {
		KEYPAD_FIFO_ORDER = 5,
	};

#if CONFIG_SERCOM0
	uint8_t m_sercom0RxFifoStor[1 << SERCOM_FIFO_ORDER];
	Fifo m_sercom0RxFifo;
#endif
	
#if CONFIG_SERCOM1
	uint8_t m_sercom1RxFifoStor[1 << SERCOM_FIFO_ORDER];
	Fifo m_sercom1RxFifo;
#endif

#if CONFIG_SERCOM2
	uint8_t m_sercom2RxFifoStor[1 << SERCOM_FIFO_ORDER];
	Fifo m_sercom2RxFifo;
#endif

#if CONFIG_SERCOM5
	uint8_t m_sercom5RxFifoStor[1 << SERCOM_FIFO_ORDER];
	Fifo m_sercom5RxFifo;
#endif

#if CONFIG_DAP
	uint8_t m_DAPRxFifoStor[1 << DAP_FIFO_ORDER];
	Fifo m_DAPRxFifo;
#endif

#if CONFIG_KEYPAD
	uint8_t m_keypadFifoStor[1 << KEYPAD_FIFO_ORDER];
	Fifo m_keypadFifo;
#endif


};


#endif // SYSTEM_H


