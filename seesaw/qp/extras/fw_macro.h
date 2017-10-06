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

#ifndef FW_MACRO_H
#define FW_MACRO_H

#include <stdint.h>
#include <string.h>

#define STRING_COPY(to_, from_, len_) \
    do { \
        strncpy((to_), (from_), (len_)); \
        if ((len_) > 0) { \
            (to_)[(len_) - 1] = '\0'; \
        } \
    } while(0)

#define CONST_STRING_LEN(a_)        (sizeof(a_) - 1)

#define ARRAY_COUNT(a_)             (sizeof(a_) / sizeof((a_)[0]))

#define LESS(a_, b_)                (((a_) < (b_)) ? (a_) : (b_))
#define GREATER(a_, b_)             (((a_) > (b_)) ? (a_) : (b_))

#define ROUND_UP_MASK(a_, m_)       (((a_) + (m_)) & ~(m_))
#define ROUND_UP_4(a_)              ROUND_UP_MASK((a_), 0x3)
#define ROUND_UP_8(a_)              ROUND_UP_MASK((a_), 0x7)
#define ROUND_UP_16(a_)             ROUND_UP_MASK((a_), 0xF)
#define ROUND_UP_32(a_)             ROUND_UP_MASK((a_), 0x1F)

#define ROUND_DOWN_MASK(a_, m_)     ((a_) & ~(m_))
#define ROUND_DOWN_4(a_)            ROUND_DOWN_MASK((a_), 0x3)
#define ROUND_DOWN_8(a_)            ROUND_DOWN_MASK((a_), 0x7)
#define ROUND_DOWN_16(a_)           ROUND_DOWN_MASK((a_), 0xF)
#define ROUND_DOWN_32(a_)           ROUND_DOWN_MASK((a_), 0x1F)

#define ROUND_UP_DIV(a_, d_)        (((a_) + (d_) - 1) / (d_))
#define ROUND_UP_DIV_4(a_)          ROUND_UP_DIV((a_), 4)
#define ROUND_UP_DIV_8(a_)          ROUND_UP_DIV((a_), 8)

#define IS_ALIGNED_MASK(a_, m_)     (((uint32_t)(a_) & (m_)) == 0)
#define IS_ALIGNED_4(a_)            IS_ALIGNED_MASK((a_), 0x3)
#define IS_ALIGNED_8(a_)            IS_ALIGNED_MASK((a_), 0x7)

#define BYTE_TO_SHORT(a1_, a0_)     (((((uint16_t)(a1_)) & 0xFF) << 8) | (((uint16_t)(a0_)) & 0xFF))
#define BYTE_TO_LONG(a3_, a2_, a1_, a0_) \
                                    (((((uint32_t)(a3_)) & 0xFF) << 24) | ((((uint16_t)(a2_)) & 0xFF) << 16) | \
                                     ((((uint32_t)(a1_)) & 0xFF) << 8) | (((uint16_t)(a0_)) & 0xFF))
#define BYTE_0(a_)                  ((a_) & 0xFF)
#define BYTE_1(a_)                  (((a_) >> 8) & 0xFF)
#define BYTE_2(a_)                  (((a_) >> 16) & 0xFF)
#define BYTE_3(a_)                  (((a_) >> 24) & 0xFF)

#define BIT_MASK_AT(b_)             (1UL << (b_))
#define BIT_MASK_UP_TO(b_)          (BIT_MASK_AT(b_) | (BIT_MASK_AT(b_) - 1))
#define BIT_MASK_OF_RANGE(b1_, b0_) ((BIT_MASK_UP_TO(b1_) & ~BIT_MASK_UP_TO(b0_)) | BIT_MASK_AT(b0_))
// This macro only works with s_ > 0
#define BIT_MASK_OF_SIZE(s_)        BIT_MASK_UP_TO((s_) - 1)

#define BIT_SET(a_, m_, b_)         ((a_) | ((m_) << (b_)))
#define BIT_CLR(a_, m_, b_)         ((a_) & ~((m_) << (b_)))
#define BIT_DEF(a_, m_, b_)         (((a_) & (m_)) << (b_))
#define BIT_READ(a_, m_, b_)        (((a_) >> (b_)) & (m_))
#define BIT_WRITE(a_, m_, b_, v_)   (BIT_CLR((a_), (m_), (b_)) | BIT_DEF((v_), (m_), (b_)))

#endif // FW_MACRO_H

