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

#include <stdarg.h>
#include <stdio.h>
#include "bsp.h"
#include "qpcpp.h"
#include "fw_pipe.h"
#include "fw_log.h"

Q_DEFINE_THIS_FILE

//#define ENABLE_LOGGING

using namespace QP;

namespace FW {

char const Log::m_truncatedError[] = "<##TRUN##>";

Fifo * Log::m_fifo = NULL;
QSignal Log::m_sig = 0;

void Log::AddInterface(Fifo *fifo, QSignal sig) {
    FW_LOG_ASSERT(fifo && sig);
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    m_fifo = fifo;
    m_sig = sig;
    QF_CRIT_EXIT(crit);
}

void Log::DeleteInterface() {
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    m_fifo = NULL;
    m_sig = 0;
    QF_CRIT_EXIT(crit);
}

void Log::Write(char const *buf, uint32_t len) {    
	/* DM:TODO: this
    if (m_fifo) {
        bool status1 = false;
        bool status2 = false;
        if (m_fifo->IsTruncated()) {
            m_fifo->WriteNoCrit(reinterpret_cast<uint8_t const *>(m_truncatedError), CONST_STRING_LEN(m_truncatedError), &status1);
        }
        if (!m_fifo->IsTruncated()) {
            m_fifo->WriteNoCrit(reinterpret_cast<uint8_t const *>(buf), len, &status2);
        }
        // Post MUST be outside critical section.
        if (status1 || status2) {
            Q_ASSERT(m_sig);
            Evt *evt = new Evt(m_sig);
            QF::PUBLISH(evt, NULL);
        }
    } else {
        // TODO remove. Test only - write to BSP usart directly.
        //BspWrite(buf, len);
    }
	*/
}

uint32_t Log::Print(char const *format, ...) {
    va_list arg;
    va_start(arg, format);
    char buf[BUF_LEN];
    uint32_t len = vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    len = LESS(len, sizeof(buf) - 1);
    Write(buf, len);
    return len;
}

void Log::Event(char const *name, char const *func, const char *evtName, int sig) {
#ifdef ENABLE_LOGGING
    Q_ASSERT(name && func && sig && evtName);
	__BKPT();
#endif
}

void Log::Debug(char const *name, char const *func, char const *format, ...) {
//DM: TODO: this
/*
    char buf[BUF_LEN];
    // Reserve 2 bytes for newline.
    const uint32_t MAX_LEN = sizeof(buf) - 2;
    // Note there is no space after type name.
    uint32_t len = snprintf(buf, MAX_LEN, "[%lu] %s (%s): ", GetSystemMs(), name, func);
    len = LESS(len, (MAX_LEN - 1));
    if (len < (MAX_LEN - 1)) {
        va_list arg;
        va_start(arg, format);
        len += vsnprintf(&buf[len], MAX_LEN - len, format, arg);
        va_end(arg);
        len = LESS(len, MAX_LEN - 1);
    }
    Q_ASSERT(len <= (sizeof(buf) - 3));
    buf[len++] = '\n';
    buf[len++] = '\r';
    buf[len] = 0;
    Write(buf, len);
*/
}

} // namespace FW
