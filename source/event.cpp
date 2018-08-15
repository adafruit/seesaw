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

#include "qpcpp.h"
#include "event.h"
#include "qp_extras.h"

Q_DEFINE_THIS_FILE

#ifdef ENABLE_LOGGING

char const * const eventName[] = {
    "NULL",
    "ENTRY",
    "EXIT",
    "INIT",
    "SYSTEM_START_REQ",
    "SYSTEM_START_CFM",
    "SYSTEM_STOP_REQ",
    "SYSTEM_STOP_CFM",
    "SYSTEM_POWER_SENSE_TIMER",
    "SYSTEM_POWER_SENSE_BLINK",
    "SYSTEM_DONE",
    "SYSTEM_FAIL",

    "USER_LED_TOGGLE_REQ",

    "DELEGATE_PROCESS_COMMAND",

    "I2C_SLAVE_START_REQ",
    "I2C_SLAVE_START_CFM",
    "I2C_SLAVE_STOP_REQ",
    "I2C_SLAVE_STOP_CFM",
    "I2C_SLAVE_REQUEST",
    "I2C_SLAVE_RECEIVE",
    "I2C_SLAVE_STOP_CONDITION",
    "I2C_SLAVE_TIMEOUT",

    "SPI_SLAVE_START_REQ",
	"SPI_SLAVE_START_CFM",
	"SPI_SLAVE_STOP_REQ",
	"SPI_SLAVE_STOP_CFM",
	"SPI_SLAVE_REQUEST",
	"SPI_SLAVE_RECEIVE",

    "DELEGATE_START_REQ",
    "DELEGATE_START_CFM",
    "DELEGATE_STOP_REQ",
    "DELEGATE_STOP_CFM",
    "DELEGATE_DATA_READY",

    "GPIO_INTERRUPT_RECEIVED",

    "ADC_START_REQ",
    "ADC_START_CFM",
    "ADC_STOP_CFM",
    "ADC_STOP_CFM",
    "ADC_READ_REG_REQ",
    "ADC_WRITE_REG_REQ",
    "ADC_WRITE_WINMON_REQ",

    "TIMER_START_REQ",
    "TIMER_START_CFM",
    "TIMER_STOP_REQ",
    "TIMER_STOP_CFM",
    "TIMER_WRITE_PWM",
    "TIMER_SET_FREQ",

    "DAC_START_REQ",
    "DAC_START_CFM",
    "DAC_STOP_REQ",
    "DAC_STOP_CFM",

    "USB_START_REQ",
	"USB_START_CFM",
	"USB_STOP_REQ",
	"USB_STOP_CFM",
    
    "DAP_START_REQ",
    "DAP_START_CFM",
    "DAP_STOP_REQ",
    "DAP_STOP_CFM",
    "DAP_REQUEST",
    "DAP_READ",

    "NEOPIXEL_START_REQ",
    "NEOPIXEL_START_CFM",
    "NEOPIXEL_STOP_REQ",
    "NEOPIXEL_STOP_CFM",
    "NEOPIXEL_SET_SPEED_REQ",
    "NEOPIXEL_SET_PIN_REQ",
    "NEOPIXEL_SET_BUFFER_REQ",
    "NEOPIXEL_SET_BUFFER_LEN_REQ",
    "NEOPIXEL_SHOW_REQ",

    "TOUCH_START_REQ",
    "TOUCH_START_CFM",
    "TOUCH_STOP_REQ",
    "TOUCH_STOP_CFM",
    "TOUCH_READ_REG_REQ",

    "INTERRUPT_START_REQ",
    "INTERRUPT_START_CFM",
    "INTERRUPT_STOP_REQ",
    "INTERRUPT_STOP_CFM",
    "INTERRUPT_SET_REQ",
    "INTERRUPT_CLEAR_REQ",

    "SERCOM_START_REQ",
    "SERCOM_START_CFM",
    "SERCOM_STOP_REQ",
    "SERCOM_STOP_CFM",
    "SERCOM_WRITE_DATA_REQ",
    "SERCOM_READ_DATA_REQ",
    "SERCOM_WRITE_REG_REQ",
    "SERCOM_READ_REG_REQ",
    "SERCOM_RX_INTERRUPT",
    "SERCOM_UART_SYNC",

    "KEYPAD_START_REQ",
	"KEYPAD_START_CFM",
	"KEYPAD_STOP_REQ",
	"KEYPAD_STOP_CFM",
	"KEYPAD_SYNC",
	"KEYPAD_WRITE_REG_REQ",
	"KEYPAD_READ_REG_REQ",
};  
  
char const * GetEvtName(QP::QSignal sig) {
    Q_ASSERT(sig < ARRAY_COUNT(eventName));
    if (sig < MAX_PUB_SIG) {
      return eventName[sig];
    }
    return "(UNKNOWN)";
}

#endif
