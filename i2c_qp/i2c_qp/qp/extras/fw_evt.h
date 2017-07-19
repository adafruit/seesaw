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

#ifndef FW_EVT_H
#define FW_EVT_H

#include "stddef.h"
#include "qpcpp.h"
#include "fw_error.h"

#define EVT_CAST(e_)            static_cast<FW::Evt const &>(e_)
#define ERROR_EVT_CAST(e_)      static_cast<FW::ErrorEvt const &>(e_)

namespace FW {

class Evt : public QP::QEvt {
public:
    static void *operator new(size_t s);
    static void operator delete(void *evt);

    Evt(QP::QSignal signal, uint16_t seq = 0) :
        QP::QEvt(signal), m_seq(seq) {}
    ~Evt() {}
    uint16_t GetSeq() const { return m_seq; }

protected:
    uint16_t m_seq;
};

class ErrorEvt : public Evt {
public:
    ErrorEvt(QP::QSignal signal, uint16_t seq = 0,
             Error error = ERROR_SUCCESS, Reason reason = 0) :
        Evt(signal, seq), m_error(error), m_reason(reason) {}

    Error GetError() const { return m_error; }
    Reason GetReason() const {return m_reason; }
protected:
    Error m_error;      // Common error code.
    Reason m_reason;    // CFM/RSP event specific reason code
};

} // namespace FW

#endif // FW_EVT_H
