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

#ifndef HSM_ID_H
#define HSM_ID_H

enum {
    SYSTEM = 1,
	DELEGATE,
	I2C_SLAVE,
	SPI_SLAVE,
	AO_DAC,
	AO_USB,
	AO_TIMER,
	AO_ADC,
	AO_SERCOM0,
	AO_SERCOM1,
	AO_SERCOM2,
	AO_SERCOM5,
	AO_INTERRUPT,
	AO_DAP,
	AO_NEOPIXEL,
};

// Higher value corresponds to higher priority.
// The maximum priority is defined in qf_port.h as QF_MAX_ACTIVE (32)
enum
{
    PRIO_SYSTEM     = 18,
	PRIO_I2C_SLAVE	= 27,
	PRIO_SPI_SLAVE  = 29,
	PRIO_USB		= 28,
	PRIO_ADC		= 24,
	PRIO_TIMER		= 25,
	PRIO_DAC		= 23,
	PRIO_SERCOM		= 26,
	PRIO_DELEGATE   = 30,
	PRIO_INTERRUPT  = 31,
	PRIO_DAP		= 21,
	PRIO_NEOPIXEL	= 20,
};


#endif // HSM_ID_H
