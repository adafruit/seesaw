/*******************************************************************************
 * Copyright (C) Lawrence Lo (https://github.com/galliumstudio). 
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

/*
* Individual events would all be defined here
*/

#ifndef EVENT_H
#define EVENT_H

#include "qpcpp.h"
#include "qp_extras.h"

#ifdef ENABLE_LOGGING
#define LOG_EVENT(e_)            Log::Event(me->m_name, __FUNCTION__, GetEvtName(e_->sig), e_->sig);
#else
#define LOG_EVENT(e_)
#endif

using namespace FW;

enum {
    SYSTEM_START_REQ = QP::Q_USER_SIG,
    SYSTEM_START_CFM,
    SYSTEM_STOP_REQ,
    SYSTEM_STOP_CFM,
    SYSTEM_TEST_TIMER,
    SYSTEM_DONE,
    SYSTEM_FAIL,
    
    USER_LED_TOGGLE_REQ,
	
	DELEGATE_PROCESS_COMMAND,
	
	I2C_SLAVE_START_REQ,
	I2C_SLAVE_START_CFM,
	I2C_SLAVE_STOP_REQ,
	I2C_SLAVE_STOP_CFM,
	I2C_SLAVE_REQUEST,
	I2C_SLAVE_RECEIVE,
	I2C_SLAVE_STOP_CONDITION,
	
	DELEGATE_START_REQ,
	DELEGATE_START_CFM,
	DELEGATE_STOP_REQ,
	DELEGATE_STOP_CFM,
	DELEGATE_DATA_READY,
    
	ADC_START_REQ,
	ADC_START_CFM,
	ADC_STOP_REQ,
	ADC_STOP_CFM,
	
	DAC_START_REQ,
	DAC_START_CFM,
	DAC_STOP_REQ,
	DAC_STOP_CFM,
	
	SERCOM_START_REQ,
	SERCOM_START_CFM,
	SERCOM_STOP_REQ,
	SERCOM_STOP_CFM,
	SERCOM_WRITE_REQ,
	SERCOM_READ,
	
    MAX_PUB_SIG
};

char const * GetEvtName(QP::QSignal sig);

class SystemStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 600
    };
    SystemStartReq(uint16_t seq) :
        Evt(SYSTEM_START_REQ, seq) {}
};

class SystemStartCfm : public ErrorEvt {
public:
    SystemStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(SYSTEM_START_CFM, seq, error, reason) {}
};

class SystemStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 600
    };
    SystemStopReq(uint16_t seq) :
        Evt(SYSTEM_STOP_REQ, seq) {}
};

class SystemStopCfm : public ErrorEvt {
public:
    SystemStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(SYSTEM_STOP_CFM, seq, error, reason) {}
};

class SystemFail : public ErrorEvt {
public:
    SystemFail(Error error, Reason reason) :
        ErrorEvt(SYSTEM_FAIL, 0, error, reason) {}
};

class UserLedToggleReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedToggleReq(uint16_t seq) :
        Evt(USER_LED_TOGGLE_REQ, seq) {}
};

//* ==========================  DELEGATE ======================== *//
class DelegateProcessCommand : public Evt {
	public:
		DelegateProcessCommand(uint8_t requesterId, uint8_t highByte, uint8_t lowByte, uint8_t rw, Fifo *fifo) :
		Evt(DELEGATE_PROCESS_COMMAND), _requesterId(requesterId), _highByte(highByte), _lowByte(lowByte), _rw(rw), _fifo(fifo)  {}
		
		uint8_t getRequesterId() const { return _requesterId; }
		uint8_t getHighByte() const { return _highByte; }
		uint8_t getLowByte() const { return _lowByte; }
		uint8_t getRw() const { return _rw; }
		Fifo *getFifo() const { return _fifo; }
	private:
		uint8_t _requesterId, _highByte, _lowByte, _rw;
		Fifo *_fifo;
};

class DelegateDataReady : public Evt {
	public:
	DelegateDataReady(uint8_t requesterId) :
	Evt(DELEGATE_DATA_READY), _requesterId(requesterId)  {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	private:
	uint8_t _requesterId;
};

class DelegateStartReq : public Evt {
	public:
	 	DelegateStartReq(Fifo *I2CSlaveOutFifo, Fifo *I2cSlaveInFifo) :
		Evt(DELEGATE_START_REQ), _i2c_slave_in_fifo(I2cSlaveInFifo), _i2c_slave_out_fifo(I2CSlaveOutFifo) {}
			
		Fifo *getI2CSlaveInFifo() const { return _i2c_slave_in_fifo; }
		Fifo *getI2CSlaveOutFifo() const { return _i2c_slave_out_fifo; }
	private:
		Fifo *_i2c_slave_in_fifo;
		Fifo *_i2c_slave_out_fifo;
};

class DelegateStartCfm : public ErrorEvt {
	public:
	DelegateStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DELEGATE_START_CFM, seq, error, reason) {}
};

class DelegateStopCfm : public ErrorEvt {
	public:
	DelegateStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DELEGATE_STOP_CFM, seq, error, reason) {}
};

//* ==========================  ADC ======================= *//

class ADCStartCfm : public ErrorEvt {
	public:
	ADCStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(ADC_START_CFM, seq, error, reason) {}
};

class ADCStopCfm : public ErrorEvt {
	public:
	ADCStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(ADC_STOP_CFM, seq, error, reason) {}
};

//* ==========================  DAC ======================= *//

class DACStartCfm : public ErrorEvt {
	public:
	DACStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DAC_START_CFM, seq, error, reason) {}
};

class DACStopCfm : public ErrorEvt {
	public:
	DACStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DAC_STOP_CFM, seq, error, reason) {}
};

//* ==========================  SERCOM ======================= *//

class SercomStartReq : public Evt {
	public:
	SercomStartReq(Fifo *rxFifo) :
	Evt(SERCOM_START_REQ), _rx_fifo(rxFifo) {}
	
	Fifo *getRxFifo() const { return _rx_fifo; }
	private:
	Fifo *_rx_fifo;
};

//TODO: add ID to these
class SERCOMStartCfm : public ErrorEvt {
	public:
	SERCOMStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(SERCOM_START_CFM, seq, error, reason) {}
};

class SERCOMStopCfm : public ErrorEvt {
	public:
	SERCOMStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(SERCOM_STOP_CFM, seq, error, reason) {}
};

class SercomWriteReq : public Evt {
	public:
	SercomWriteReq(uint8_t requesterId, Fifo *source) :
	Evt(SERCOM_WRITE_REQ), _source(source) {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	Fifo *getSource() const { return _source; }
	private:
	uint8_t _requesterId;
	Fifo *_source;
};

//* ==========================  I2C SLAVE ======================= *//

class I2CSlaveStartReq : public Evt {
	public:
	I2CSlaveStartReq(Fifo *outFifo, Fifo *inFifo) :
	Evt(I2C_SLAVE_START_REQ), _in_fifo(inFifo), _out_fifo(outFifo) {}
	
	Fifo *getInFifo() const { return _in_fifo; }
	Fifo *getOutFifo() const { return _out_fifo; }
	private:
	Fifo *_in_fifo;
	Fifo *_out_fifo;
};

class I2CSlaveStartCfm : public ErrorEvt {
	public:
	I2CSlaveStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(I2C_SLAVE_START_CFM, seq, error, reason) {}
};

class I2CSlaveStopCfm : public ErrorEvt {
	public:
	I2CSlaveStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(I2C_SLAVE_STOP_CFM, seq, error, reason) {}
};

#endif