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

#ifndef FW_PIPE_H
#define FW_PIPE_H

#include "fw_macro.h"
#include "fw_error.h"

#define FW_PIPE_ASSERT(t_) ((t_) ? (void)0 : Q_onAssert("fw_pipe.h", (int_t)__LINE__))

namespace FW {

// Critical sections are enforced internally.
template <class Type>
class Pipe {
public:
    Pipe(Type stor[], uint8_t order) :
        m_stor(stor), m_mask(BIT_MASK_OF_SIZE(order)),
        m_writeIndex(0), m_readIndex(0), m_truncated(false) {
        // Arithmetic in this class (m_mask + 1) assumes order < 32.
        // BIT_MASK_OF_SIZE() assumes order > 0
        FW_PIPE_ASSERT(stor && (order > 0) and (order < 32));
    }
    virtual ~Pipe() {}

    void Reset() {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        m_writeIndex = 0;
        m_readIndex = 0;
        m_truncated = false;
        QF_CRIT_EXIT(crit);
    }
    bool IsTruncated() const { return m_truncated; }
    uint32_t GetWriteIndex() const { return m_writeIndex; }
    uint32_t GetReadIndex() const { return m_readIndex; }
    uint32_t GetUsedCount() const {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        uint32_t count = GetUsedCountNoCrit();
        QF_CRIT_EXIT(crit);
        return count;
    }
    uint32_t GetUsedCountNoCrit() const {
        return (m_writeIndex - m_readIndex) & m_mask;
    }
    uint32_t GetAvailCount() const {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        uint32_t count = GetAvailCountNoCrit();
        QF_CRIT_EXIT(crit);
        return count;
    }
    // Since (m_readIndex == m_writeIndex) is regarded as empty, the maximum available count =
    // total storage - 1, i.e. m_mask.
    uint32_t GetAvailCountNoCrit() const {
        return (m_readIndex - m_writeIndex - 1) & m_mask;
    }
    uint32_t GetDiff(uint32_t a, uint32_t b) { return (a - b) & m_mask; }
    uint32_t GetAddr(uint32_t index) { return reinterpret_cast<uint32_t>(&m_stor[index & m_mask]); }
    uint32_t GetWriteAddr() { return GetAddr(m_writeIndex); }
    uint32_t GetReadAddr() { return GetAddr(m_readIndex); }
    uint32_t GetMaxAddr() { return GetAddr(m_mask); }
    void IncWriteIndex(uint32_t count) {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        IncIndex(m_writeIndex, count);
        QF_CRIT_EXIT(crit);
    }
    void IncReadIndex(uint32_t count) {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        IncIndex(m_readIndex, count);
        QF_CRIT_EXIT(crit);
    }

    // Return written count. If not enough space to write all, return 0 (i.e. no partial write).
    // If overflow has occurred set m_truncated; otherwise clear m_truncated.
    uint32_t Write(Type const *src, uint32_t count, bool *status = NULL) {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        count = WriteNoCrit(src, count, status);
        QF_CRIT_EXIT(crit);
        return count;
    }

    // Without critical section.
    uint32_t WriteNoCrit(Type const *src, uint32_t count, bool *status = NULL) {
        FW_PIPE_ASSERT(src);
        bool wasEmpty = IsEmpty();
        if (count > GetAvailCountNoCrit()) {
            m_truncated = true;
            count = 0;
        } else {
            m_truncated = false;
            if ((m_writeIndex + count) <= (m_mask + 1)) {
                WriteBlock(src, count);
            } else {
                uint32_t partial = m_mask + 1 - m_writeIndex;
                WriteBlock(src, partial);
                WriteBlock(src + partial, count - partial);
            }
        }
        if (status) {
            if (count && wasEmpty) {
                *status = true;
            } else {
                *status = false;
            }
        }
        return count;
    }

    // Return actual read count. Okay if data in pipe < count.
    uint32_t Read(Type *dest, uint32_t count, bool *status = NULL) {
        QF_CRIT_STAT_TYPE crit;
        QF_CRIT_ENTRY(crit);
        count = ReadNoCrit(dest, count, status);
        QF_CRIT_EXIT(crit);
        return count;
    }

    // Without critical section.
    uint32_t ReadNoCrit(Type *dest, uint32_t count, bool *status = NULL) {
        FW_PIPE_ASSERT(dest);
        uint32_t used = GetUsedCountNoCrit();
        count = LESS(count, used);
        if ((m_readIndex + count) <= (m_mask + 1)) {
            ReadBlock(dest, count);
        } else {
            uint32_t partial = m_mask + 1 - m_readIndex;
            ReadBlock(dest, partial);
            ReadBlock(dest + partial, count - partial);
        }
        if (status) {
            if (count && IsEmpty()) {
                // Currently use "empty" as condition, but it can be half-empty, etc.
                *status = true;
            } else {
                *status = 0;
            }
        }
        return count;
    }

protected:
    // Write contiguous block to m_stor. count can be 0.
    void WriteBlock(Type const *src, uint32_t count) {
        FW_PIPE_ASSERT(src && ((m_writeIndex + count) <= (m_mask + 1)));
        for (uint32_t i = 0; i < count; i++) {
            m_stor[m_writeIndex + i] = src[i];
        }
        IncIndex(m_writeIndex, count);
    }
    // Read contiguous block from m_stor. count can be 0.
    void ReadBlock(Type *dest, uint32_t count) {
        FW_PIPE_ASSERT(dest && ((m_readIndex + count) <= (m_mask + 1)));
        for (uint32_t i = 0; i < count; i++) {
            dest[i] = m_stor[m_readIndex + i];
        }
        IncIndex(m_readIndex, count);
    }
    void IncIndex(uint32_t &index, uint32_t count) {
        index = (index + count) & m_mask;
    }
    bool IsEmpty() {
        return (m_readIndex == m_writeIndex);
    }

    Type *      m_stor;
    uint32_t    m_mask;
    uint32_t    m_writeIndex;
    uint32_t    m_readIndex;
    bool        m_truncated;
    // For future enhancement.
    //QP::QSignal m_halfEmptySig; // signal to send when pipe has just crossed half-empty threshold upon read.

    // Unimplemented to disallow built-in memberwise copy constructor and assignment operator.
    Pipe(Pipe const &);
    Pipe& operator= (Pipe const &);
};

// Common template instantiation
typedef Pipe<uint8_t> Fifo;

} // namespace FW

#endif //FW_PIPE_H
