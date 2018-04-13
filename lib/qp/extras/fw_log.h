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

#ifndef FW_LOG_H
#define FW_LOG_H

#include "qpcpp.h"
#include "fw_pipe.h"

#define FW_LOG_ASSERT(t_) ((t_) ? (void)0 : Q_onAssert("fw_log.h", (int_t)__LINE__))

namespace FW {

#define PRINT(format_, ...)      Log::Print(format_, ## __VA_ARGS__)
// The following macros can only be used within an HSM. Newline is automatically appended.
#define DEBUG(format_, ...)      Log::Debug(me->m_name, __FUNCTION__, format_, ## __VA_ARGS__);

class Log {
public:
    static void AddInterface(Fifo *fifo, QP::QSignal sig);
    static void DeleteInterface();
    static void Write(char const *buf, uint32_t len);
    static uint32_t Print(char const *format, ...);
    static void Event(char const *name, char const *func, const char *evtName, int sig);
    static void Debug(char const *name, char const *func, char const *format, ...);
    
private:

    enum {
        BUF_LEN = 160,
    };

    static Fifo *m_fifo;
    static QP::QSignal m_sig;
    static char const m_truncatedError[];
};

} // namespace FW

#endif // FW_LOG_H
