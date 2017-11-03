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
	I2C_SLAVE_TIMEOUT,
	
	DELEGATE_START_REQ,
	DELEGATE_START_CFM,
	DELEGATE_STOP_REQ,
	DELEGATE_STOP_CFM,
	DELEGATE_DATA_READY,
	
	GPIO_INTERRUPT_RECEIVED,
    
	ADC_START_REQ,
	ADC_START_CFM,
	ADC_STOP_REQ,
	ADC_STOP_CFM,
	ADC_READ_REG_REQ,
	ADC_WRITE_REG_REQ,
	ADC_WRITE_WINMON_REQ,
	
	TIMER_START_REQ,
	TIMER_START_CFM,
	TIMER_STOP_REQ,
	TIMER_STOP_CFM,
	TIMER_WRITE_PWM,
	TIMER_SET_FREQ,
	
	DAC_START_REQ,
	DAC_START_CFM,
	DAC_STOP_REQ,
	DAC_STOP_CFM,
	
	DAP_START_REQ,
	DAP_START_CFM,
	DAP_STOP_REQ,
	DAP_STOP_CFM,
	DAP_REQUEST,
	DAP_READ,
	
	NEOPIXEL_START_REQ,
	NEOPIXEL_START_CFM,
	NEOPIXEL_STOP_REQ,
	NEOPIXEL_STOP_CFM,
	NEOPIXEL_SET_SPEED_REQ,
	NEOPIXEL_SET_PIN_REQ,
	NEOPIXEL_SET_BUFFER_REQ,
	NEOPIXEL_SET_BUFFER_LEN_REQ,
	NEOPIXEL_SHOW_REQ,
	
	INTERRUPT_START_REQ,
	INTERRUPT_START_CFM,
	INTERRUPT_STOP_REQ,
	INTERRUPT_STOP_CFM,
	INTERRUPT_SET_REQ,
	INTERRUPT_CLEAR_REQ,
	
	SERCOM_START_REQ,
	SERCOM_START_CFM,
	SERCOM_STOP_REQ,
	SERCOM_STOP_CFM,
	SERCOM_WRITE_DATA_REQ,
	SERCOM_READ_DATA_REQ,
	SERCOM_WRITE_REG_REQ,
	SERCOM_READ_REG_REQ,
	SERCOM_RX_INTERRUPT,
	
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
		DelegateProcessCommand(uint8_t requesterId, uint8_t highByte, uint8_t lowByte, uint8_t len, Fifo *fifo) :
		Evt(DELEGATE_PROCESS_COMMAND), _requesterId(requesterId), _highByte(highByte), _lowByte(lowByte), _len(len), _fifo(fifo)  {}
		
		uint8_t getRequesterId() const { return _requesterId; }
		uint8_t getHighByte() const { return _highByte; }
		uint8_t getLowByte() const { return _lowByte; }
		uint8_t getLen() const { return _len; }
		Fifo *getFifo() const { return _fifo; }
	private:
		uint8_t _requesterId, _highByte, _lowByte, _len;
		Fifo *_fifo;
};

class DelegateDataReady : public Evt {
	public:
	DelegateDataReady(uint8_t requesterId)  :
	Evt(DELEGATE_DATA_READY), _requesterId(requesterId), _fifo(NULL)  {}
		
	DelegateDataReady(uint8_t requesterId, Fifo *fifo) :
	Evt(DELEGATE_DATA_READY), _requesterId(requesterId), _fifo(fifo)  {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	Fifo *getFifo() const { return _fifo; }
	private:
	uint8_t _requesterId;
	Fifo *_fifo;
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

class ADCReadRegReq : public Evt {
	public:
	ADCReadRegReq(uint8_t requesterId, uint8_t reg, Fifo *dest) :
	Evt(ADC_READ_REG_REQ), _requesterId(requesterId), _reg(reg), _dest(dest) {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	uint8_t getReg() const { return _reg; }
	Fifo *getDest() const { return _dest; }
	
	private:
	uint8_t _requesterId, _reg;
	Fifo *_dest;
};

class ADCWriteRegReq : public Evt {
	public:
	ADCWriteRegReq(uint8_t reg, uint8_t value) :
	Evt(ADC_WRITE_REG_REQ), _reg(reg), _value(value) {}
	
	uint8_t getReg() const { return _reg; }
	uint8_t getValue() const { return _value; }
	
	private:
	uint8_t _reg, _value;
};

class ADCWriteWinmonThresh : public Evt {
	public:
	ADCWriteWinmonThresh(uint16_t upper, uint16_t lower) : 
	Evt(ADC_WRITE_WINMON_REQ), _upper(upper), _lower(lower) {}
		
	uint16_t getUpper() const { return _upper; }
	uint16_t getLower() const { return _lower; }
		
	private:
	uint16_t _upper, _lower;
};

//* ==========================  Timer ======================= *//

class TimerStartCfm : public ErrorEvt {
	public:
	TimerStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(TIMER_START_CFM, seq, error, reason) {}
};

class TimerStopCfm : public ErrorEvt {
	public:
	TimerStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(TIMER_STOP_CFM, seq, error, reason) {}
};

class TimerWritePWM : public Evt {
	public:
	TimerWritePWM(uint8_t pwm, uint16_t value) :
	Evt(TIMER_WRITE_PWM), _pwm(pwm), _value(value) {}
		
	uint8_t getPwm() const { return _pwm; }
	uint16_t getValue() const { return _value; }
		
	private:
	uint8_t _pwm;
	uint16_t _value;
};

class TimerSetFreq : public Evt {
	public:
	TimerSetFreq(uint8_t pwm, uint8_t freq) :
	Evt(TIMER_SET_FREQ), _pwm(pwm), _freq(freq) {}
	
	uint8_t getPwm() const { return _pwm; }
	uint8_t getFreq() const { return _freq; }
	
	private:
	uint8_t _pwm, _freq;
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

//* ==========================  DAP ======================= *//

class DAPStartReq : public Evt {
	public:
	DAPStartReq(Fifo *rxFifo) :
	Evt(DAP_START_REQ), _rx_fifo(rxFifo) {}
	
	Fifo *getRxFifo() const { return _rx_fifo; }
	private:
	Fifo *_rx_fifo;
};

class DAPStartCfm : public ErrorEvt {
	public:
	DAPStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DAP_START_CFM, seq, error, reason) {}
};

class DAPStopCfm : public ErrorEvt {
	public:
	DAPStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(DAP_STOP_CFM, seq, error, reason) {}
};

class DAPRequest : public Evt {
	public:
	DAPRequest(uint8_t requesterId, Fifo *source, uint8_t len) :
	Evt(DAP_REQUEST), _requesterId(requesterId), _source(source), _len(len) {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	Fifo *getSource() const { return _source; }
	uint8_t getLen() const { return _len; }
	private:
	uint8_t _requesterId;
	Fifo *_source;
	uint8_t _len;
};

class DAPRead : public Evt {
	public:
	DAPRead(uint8_t requesterId) :
	Evt(DAP_READ), _requesterId(requesterId) {}
	
	uint8_t getRequesterId() const { return _requesterId; }
	private:
	uint8_t _requesterId;
};

//* ==========================  Neopixel ======================= *//

class NeopixelStartCfm : public ErrorEvt {
	public:
	NeopixelStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(NEOPIXEL_START_CFM, seq, error, reason) {}
};

class NeopixelStopCfm : public ErrorEvt {
	public:
	NeopixelStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(NEOPIXEL_STOP_CFM, seq, error, reason) {}
};

class NeopixelSetPinReq : public Evt {
	public:
	NeopixelSetPinReq(uint8_t pin) :
	Evt(NEOPIXEL_SET_PIN_REQ), _pin(pin) {}
	
	uint8_t getPin() const { return _pin; }
	private:
	uint8_t _pin;
};

class NeopixelSetSpeedReq : public Evt {
	public:
	NeopixelSetSpeedReq(uint8_t speed) :
	Evt(NEOPIXEL_SET_SPEED_REQ), _speed(speed) {}
	
	uint8_t getSpeed() const { return _speed; }
	private:
	uint8_t _speed;
};

class NeopixelSetBufferLengthReq : public Evt {
	public:
	NeopixelSetBufferLengthReq(uint16_t len) :
	Evt(NEOPIXEL_SET_BUFFER_LEN_REQ), _len(len) {}
	
	uint8_t getLen() const { return _len; }
	private:
	uint8_t _len;
};

class NeopixelSetBufferReq : public Evt {
	public:
	NeopixelSetBufferReq(uint16_t addr, Fifo *source) :
	Evt(NEOPIXEL_SET_BUFFER_REQ), _addr(addr), _source(source){}
	
	uint16_t getAddr() const { return _addr; }
	Fifo *getSource() const { return _source; }
	private:
	uint16_t _addr;
	Fifo *_source;
};

//* ==========================  Interrupt ======================= *//

class InterruptStartCfm : public ErrorEvt {
	public:
	InterruptStartCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(INTERRUPT_START_CFM, seq, error, reason) {}
};

class InterruptStopCfm : public ErrorEvt {
	public:
	InterruptStopCfm(uint16_t seq, Error error, Reason reason = 0) :
	ErrorEvt(INTERRUPT_STOP_CFM, seq, error, reason) {}
};

class InterruptSetReq : public Evt {
	public:
	InterruptSetReq(uint32_t id) : 
	Evt(INTERRUPT_SET_REQ), _id(id) {}
	
	uint32_t getId() const { return _id; }
	private:
	uint32_t _id;	
};

class InterruptClearReq : public Evt {
	public:
	InterruptClearReq(uint32_t id) :
	Evt(INTERRUPT_CLEAR_REQ), _id(id) {}
	
	uint32_t getId() const { return _id; }
	private:
	uint32_t _id;
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

class SercomWriteDataReq : public Evt {
	public:
	SercomWriteDataReq(Fifo *source, uint8_t len) :
	Evt(SERCOM_WRITE_DATA_REQ), _source(source), _len(len){}
	
	Fifo *getSource() const { return _source; }
	uint8_t getLen() const { return _len; }
	private:
	Fifo *_source;
	uint8_t _len;
};

class SercomReadDataReq : public Evt {
	public:
	SercomReadDataReq(uint8_t requesterId) : 
	Evt(SERCOM_READ_DATA_REQ), _requesterId(requesterId) {}
		
	uint8_t getRequesterId() const { return _requesterId; }
	private:
	uint8_t _requesterId;
	
};

class SercomReadRegReq : public Evt {
	public:
	SercomReadRegReq(uint8_t requesterId, uint8_t reg, Fifo *dest) : 
	Evt(SERCOM_READ_REG_REQ), _requesterId(requesterId), _reg(reg), _dest(dest) {}
		
	uint8_t getRequesterId() const { return _requesterId; }
	uint8_t getReg() const { return _reg; }
	Fifo *getDest() const { return _dest; }
		
	private:
	uint8_t _requesterId, _reg;
	Fifo *_dest;	
};

class SercomWriteRegReq : public Evt {
	public:
	SercomWriteRegReq(uint8_t reg, uint32_t value) :
	Evt(SERCOM_WRITE_REG_REQ), _reg(reg), _value(value) {}
	
	uint8_t getReg() const { return _reg; }
	uint32_t getValue() const { return _value; }
	
	private:
	uint8_t _reg;
	uint32_t _value;
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

class I2CSlaveReceive : public Evt {
	public:
	I2CSlaveReceive(uint8_t highByte, uint8_t lowByte, uint8_t len) :
	Evt(I2C_SLAVE_RECEIVE), _highByte(highByte), _lowByte(lowByte), _len(len) {}
		
	uint8_t getHighByte() const { return _highByte; }
	uint8_t getLowByte() const { return _lowByte; }
	uint8_t getLen() const { return _len; }
		
	private:
	uint8_t _highByte, _lowByte, _len;
};

#endif