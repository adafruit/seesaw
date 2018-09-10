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

#include "qpcpp.h"
#include "qp_extras.h"

#include "hsm_id.h"
#include "AOUSB.h"
#include "event.h"

#include "USB/USBCore.h"
#include "USB/USBAPI.h"
#include "USB/USBDesc.h"

#include "sam.h"
#include "bsp_dma.h"

#include "SeesawConfig.h"

Q_DEFINE_THIS_FILE

using namespace FW;

static Fifo *m_inFifo;
//static Fifo *m_outFifo;

#ifdef USB_UART_DMA
static uint8_t DMA_OUT_BUF[1024];
#endif

AOUSB::AOUSB() :
    QActive((QStateHandler)&AOUSB::InitialPseudoState), 
    m_id(AO_USB), m_name("USB") {}

QState AOUSB::InitialPseudoState(AOUSB * const me, QEvt const * const e) {
    (void)e;
    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(USB_START_REQ);
    me->subscribe(USB_STOP_REQ);
      
    return Q_TRAN(&AOUSB::Root);
}

QState AOUSB::Root(AOUSB * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&AOUSB::Stopped);
            break;
        }
		case USB_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOUSB::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOUSB::Stopped(AOUSB * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case USB_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new USBStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case USB_START_REQ: {
            LOG_EVENT(e);
			
			USBStartReq const &req = static_cast<USBStartReq const &>(*e);
			m_inFifo = req.getInFifo();
			//m_outFifo = req.getOutFifo(); //not needed right now
			
#ifdef USB_UART_DMA
            dmac_alloc(CONFIG_USB_UART_DMA_CHANNEL_TX);
            dmac_set_action(CONFIG_USB_UART_DMA_CHANNEL_TX, DMA_TRIGGER_ACTON_BEAT);
            dmac_set_trigger(CONFIG_USB_UART_DMA_CHANNEL_TX, CONFIG_USB_UART_DMA_TRIGGER_TX);
#endif

			m_inFifo->Reset();
			//m_outFifo->Reset();
			
			USBDevice.init();
			USBDevice.attach();
		
			Evt *evt = new USBStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOUSB::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOUSB::Root);
            break;
        }
    }
    return status;
}

QState AOUSB::Started(AOUSB * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case USB_START_REQ: {
            LOG_EVENT(e);
            USBStartReq const &req = static_cast<USBStartReq const &>(*e);
            Evt *evt = new USBStartCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);

            status = Q_HANDLED();
            break;
        }
		case USB_STOP_REQ: {
			LOG_EVENT(e);
            
            Evt const &req = EVT_CAST(*e);
			Evt *evt = new USBStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);

#if 0       //do not restart usb on soft reboot
			USBDevice.detach();
			status = Q_TRAN(AOUSB::Stopped);
#else  
            status = Q_HANDLED();
#endif
			break;
		}
        default: {
            status = Q_SUPER(&AOUSB::Root);
            break;
        }
    }
    return status;
}

void AOUSB::ReceiveCallback(){
#ifdef USB_UART_DMA
	uint32_t len = USBDevice.available(CDC_ENDPOINT_OUT);
    if(len > 0){

        Q_ASSERT(len < sizeof(DMA_OUT_BUF));

        while(dmac_is_active(CONFIG_USB_UART_DMA_CHANNEL_TX));

        while(USBDevice.recv(CDC_ENDPOINT_OUT, DMA_OUT_BUF, len) < 0);

        dmac_set_descriptor(
            CONFIG_USB_UART_DMA_CHANNEL_TX,
            (void *)DMA_OUT_BUF,
            (void *)&CONFIG_USB_UART_SERCOM->USART.DATA.reg,
            len,
            DMA_BEAT_SIZE_BYTE,
            true,
            false);

        dmac_start(CONFIG_USB_UART_DMA_CHANNEL_TX);
    }
#elif defined(USB_UART_DIRECT)
	//put the data into the fifo
	uint8_t len = 0;
	while(USBDevice.available(CDC_ENDPOINT_OUT)){
		uint8_t rec = USBDevice.recv(CDC_ENDPOINT_OUT);
		m_inFifo->Write(&rec, 1);
		len++;
	}
	
	Evt *evt = new SercomWriteDataReq(m_inFifo, len);
	QF::PUBLISH(evt, 0);
#endif
}

void AOUSB::setBaudRate(uint32_t baud)
{
#if defined(USB_UART_DMA) || defined(USB_UART_DIRECT)
	Evt *evt = new SercomWriteRegReq(SEESAW_SERCOM_BAUD, baud);
	QF::PUBLISH(evt, 0);
#endif
}
