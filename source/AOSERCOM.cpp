﻿/*******************************************************************************
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
#include "AOSERCOM.h"
#include "event.h"

#include "SeesawConfig.h"
#include "RegisterMap.h"
#include "bsp_sercom.h"
#include "bsp_gpio.h"
//#include "bsp_dma.h"

//for direct UART<->USB
#include "USB/USBCore.h"
#include "USB/USBAPI.h"
#include "USB/USBDesc.h"

Q_DEFINE_THIS_FILE

using namespace FW;

#ifdef USB_UART_DMA
static uint8_t DMA_IN_BUF[2][64];
static bool dma_ix = 0;
#endif

//TODO: we probably will need these for each sercom
static Fifo *m_rxFifo;

AOSERCOM::AOSERCOM( Sercom *sercom, uint8_t id, uint8_t offset ) :
    QActive((QStateHandler)&AOSERCOM::InitialPseudoState),
#ifdef USB_UART_DMA
	m_syncTimer(this, SERCOM_UART_SYNC),
#endif
    m_id(id), m_name("SERCOM"), m_sercom(sercom), m_offset(offset) {}

QState AOSERCOM::InitialPseudoState(AOSERCOM * const me, QEvt const * const e) {
    (void)e;
#ifdef USB_UART_DMA
	me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));
	me->subscribe(SERCOM_UART_SYNC);
#endif

    me->subscribe(SERCOM_START_REQ);
    me->subscribe(SERCOM_STOP_REQ);
	
	me->subscribe(SERCOM_WRITE_DATA_REQ);
	me->subscribe(SERCOM_READ_DATA_REQ);
	me->subscribe(SERCOM_READ_REG_REQ);
	me->subscribe(SERCOM_WRITE_REG_REQ);
	
	me->subscribe(SERCOM_RX_INTERRUPT);
      
    return Q_TRAN(&AOSERCOM::Root);
}

QState AOSERCOM::Root(AOSERCOM * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOSERCOM::Stopped);
            break;
        }
		case SERCOM_STOP_REQ: {
			LOG_EVENT(e);
			status = Q_TRAN(&AOSERCOM::Stopped);
			break;
		}
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState AOSERCOM::Stopped(AOSERCOM * const me, QEvt const * const e) {
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
        case SERCOM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SERCOMStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SERCOM_START_REQ: {
            LOG_EVENT(e);
			
			SercomStartReq const &req = static_cast<SercomStartReq const &>(*e);
			m_rxFifo = req.getRxFifo();
			
			me->m_status.set(0x00);
			me->m_inten.set(0x00);
			me->m_intenclr.set(0x00);
			
			me->m_baud = CONFIG_SERCOM_UART_BAUD_RATE;
			
			Evt const &r = EVT_CAST(*e);
			Evt *evt = new SERCOMStartCfm(r.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
			
			status = Q_TRAN(&AOSERCOM::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOSERCOM::Root);
            break;
        }
    }
    return status;
}

QState AOSERCOM::Started(AOSERCOM * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
			
            status = Q_HANDLED();
            break;
        }
		case Q_INIT_SIG: {
			status = Q_TRAN(&AOSERCOM::UART);
			break;
		}
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
		case SERCOM_STOP_REQ: {
			LOG_EVENT(e);
			Evt const &req = EVT_CAST(*e);
			Evt *evt = new SERCOMStopCfm(req.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);
#ifdef USB_UART_DMA
			//hacky but don't restart if we're doing USB to UART DMA
			status = Q_HANDLED();
#else
			status = Q_TRAN(AOSERCOM::Stopped);
#endif
			break;
		}
        default: {
            status = Q_SUPER(&AOSERCOM::Root);
            break;
        }
    }
    return status;
}

QState AOSERCOM::UART(AOSERCOM * const me, QEvt const * const e) {
	QState status;
	switch (e->sig) {
		case Q_ENTRY_SIG: {
			LOG_EVENT(e);
			
			//set up UART
			pinPeripheral(CONFIG_SERCOM_UART_PIN_RX, CONFIG_SERCOM_UART_MUX_RX);
			pinPeripheral(CONFIG_SERCOM_UART_PIN_TX, CONFIG_SERCOM_UART_MUX_TX);
			
			initUART(me->m_sercom, SAMPLE_RATE_x16, me->m_baud);
			initFrame(me->m_sercom, CONFIG_SERCOM_UART_CHAR_SIZE, LSB_FIRST, CONFIG_SERCOM_UART_PARITY, CONFIG_SERCOM_UART_STOP_BIT);
			initPads(me->m_sercom, CONFIG_SERCOM_UART_PAD_TX, CONFIG_SERCOM_UART_PAD_RX);

#ifdef USB_UART_DMA
			me->m_syncTimer.armX(5, 5);
			disableInterruptsUART(me->m_sercom);
			dmac_init();

            dmac_alloc(CONFIG_USB_UART_DMA_CHANNEL_RX);
            dmac_set_action(CONFIG_USB_UART_DMA_CHANNEL_RX, DMA_TRIGGER_ACTON_BEAT);
            dmac_set_trigger(CONFIG_USB_UART_DMA_CHANNEL_RX, CONFIG_USB_UART_DMA_TRIGGER_RX);

            dmac_set_descriptor(
              CONFIG_USB_UART_DMA_CHANNEL_RX,
              (void *)&me->m_sercom->USART.DATA.reg,
              (void *)DMA_IN_BUF[0],
              sizeof(DMA_IN_BUF[0]),
              DMA_BEAT_SIZE_BYTE,
              false,
              true);
			dmac_start(CONFIG_USB_UART_DMA_CHANNEL_RX);
#endif

			enableUART(me->m_sercom);
			
			status = Q_HANDLED();
			break;
		}
		
		case SERCOM_WRITE_DATA_REQ: {
			
			SercomWriteDataReq const &req = static_cast<SercomWriteDataReq const &>(*e);
			Fifo *source = req.getSource();
			
			int len = req.getLen();
			uint8_t c = 0;
			for(int i=len; i>0; i--){
				source->Read(&c, 1);
				writeDataUART(me->m_sercom, c);
			}
			
			status = Q_HANDLED();
			break;	
		}
		case SERCOM_READ_DATA_REQ: {
			
			//clear DATA_RDY flag
			me->m_status.DATA_RDY = 0;
			
			if(me->m_inten.DATA_RDY){
				//post an interrupt event
				Evt *evt = new InterruptClearReq( (SEESAW_INTERRUPT_SERCOM0_DATA_RDY << me->m_offset) );
				QF::PUBLISH(evt, me);
			}
			
			SercomReadDataReq const &req = static_cast<SercomReadDataReq const &>(*e);
			Evt *evt = new DelegateDataReady(req.getRequesterId(), m_rxFifo);
			QF::PUBLISH(evt, me);
			status = Q_HANDLED();
			break;	
		}
		case SERCOM_READ_REG_REQ: {
			SercomReadRegReq const &req = static_cast<SercomReadRegReq const &>(*e);
			Fifo *dest = req.getDest();
			uint8_t reg = req.getReg();
			
			//there should be nothing in the destination pipe
			Q_ASSERT(!dest->GetUsedCount());
			
			Evt *evt;
			uint8_t c;
			switch (reg){
				case SEESAW_SERCOM_STATUS:{
					c = me->m_status.get();
					break;
				}
				case SEESAW_SERCOM_INTEN:{
					c = me->m_inten.get();
					break;
				}
				default:
					//TODO: lets handle this error better
					Q_ASSERT(0);
					break;
			}
			
			dest->Write(&c, 1);
			evt = new DelegateDataReady(req.getRequesterId());
			QF::PUBLISH(evt, me);
			
			status = Q_HANDLED();
			break;
		}
		case SERCOM_WRITE_REG_REQ: {
			SercomWriteRegReq const &req = static_cast<SercomWriteRegReq const &>(*e);
			uint8_t reg = req.getReg();
			
			switch (reg){
				case SEESAW_SERCOM_INTEN:{
					me->m_inten.set(req.getValue());
					break;
				}
				case SEESAW_SERCOM_BAUD:{
					me->m_baud = req.getValue();
					setUARTBaud(me->m_sercom, me->m_baud);
					break;
				}
				default:
				//TODO: lets handle this error better
				Q_ASSERT(0);
				break;
			}
			
			status = Q_HANDLED();
			break;
		}
#ifdef USB_UART_DMA
		case SERCOM_UART_SYNC: {
			//TODO: disable interrupts
			
			dmac_abort(CONFIG_USB_UART_DMA_CHANNEL_RX);
			uint8_t count = dmac_get_transfer_count(CONFIG_USB_UART_DMA_CHANNEL_RX);
			if(count > 0){
				//swap the buffers and send the data to USB if anything has been received
				USBDevice.send(CDC_ENDPOINT_IN, DMA_IN_BUF[dma_ix], count);
				dma_ix = !dma_ix;

				dmac_set_descriptor(
					CONFIG_USB_UART_DMA_CHANNEL_RX,
					(void *)&me->m_sercom->USART.DATA.reg,
					(void *)DMA_IN_BUF[dma_ix],
					sizeof(DMA_IN_BUF[dma_ix]),
					DMA_BEAT_SIZE_BYTE,
					false,
					true);
			}
			dmac_start(CONFIG_USB_UART_DMA_CHANNEL_RX);

			status = Q_HANDLED();
			break;
		}
#endif
		case SERCOM_RX_INTERRUPT: {
#ifdef USB_UART_DIRECT
			//messy but just pipe this right out to USB for dsp feather
			uint8_t toUSB[64];
			uint8_t bytesRead = m_rxFifo->Read(toUSB, m_rxFifo->GetUsedCount());
			
			USBDevice.send(CDC_ENDPOINT_IN, toUSB, bytesRead);
#else
			me->m_status.DATA_RDY = 1;
			if(me->m_inten.DATA_RDY){
				//post an interrupt event
				Evt *evt = new InterruptSetReq( (SEESAW_INTERRUPT_SERCOM0_DATA_RDY << me->m_offset) );
				QF::PUBLISH(evt, me);
			}
#endif
			status = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			LOG_EVENT(e);
#ifdef USB_UART_DMA
			me->m_syncTimer.disarm();
#endif
			resetUART( me->m_sercom );
			status = Q_HANDLED();
			break;
		}

#ifdef USB_UART_DMA
		case SERCOM_START_REQ:{
			Evt const &r = EVT_CAST(*e);
			Evt *evt = new SERCOMStartCfm(r.GetSeq(), ERROR_SUCCESS);
			QF::PUBLISH(evt, me);

			status = Q_HANDLED();
			break;
		}
#endif

		default: {
			status = Q_SUPER(&AOSERCOM::Started);
			break;
		}
	}
	return status;
}

QState AOSERCOM::SPI(AOSERCOM * const me, QEvt const * const e) {
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
		
		default: {
			status = Q_SUPER(&AOSERCOM::Started);
			break;
		}
	}
	return status;
}

void AOSERCOM::RxCallback(){
	Evt *evt = new Evt(SERCOM_RX_INTERRUPT);
	QF::PUBLISH(evt, 0);
}

extern "C" {
	
void sercom_handler( Sercom * sercom )
{
	if( isEnabledUART( sercom ) ){
		if (availableDataUART( sercom )) {
			uint8_t c = readDataUART( sercom );
#ifndef USB_UART_DIRECT
			m_rxFifo->Write(&c, 1);
			AOSERCOM::RxCallback();
#else		
			USBDevice.send(CDC_ENDPOINT_IN, &c, 1);
#endif
		}

		if (isUARTError( sercom )) {
			acknowledgeUARTError( sercom );
			// TODO: if (sercom->isBufferOverflowErrorUART()) ....
			// TODO: if (sercom->isFrameErrorUART()) ....
			// TODO: if (sercom->isParityErrorUART()) ....
			clearStatusUART( sercom );
		}
	}
	//TODO: else if ( isEnabledSPI( sercom ) ) { }
}
#if defined(SERCOM0) && CONFIG_SERCOM0
void SERCOM0_Handler(void){
	QXK_ISR_ENTRY();
	sercom_handler(SERCOM0);
	QXK_ISR_EXIT();
}
#endif
#if defined(SERCOM1) && CONFIG_SERCOM1
void SERCOM1_Handler(void){
	QXK_ISR_ENTRY();
	sercom_handler(SERCOM1);
	QXK_ISR_EXIT();
}
#endif
#if defined(SERCOM2) && CONFIG_SERCOM2
void SERCOM2_Handler(void){
	QXK_ISR_ENTRY();
	sercom_handler(SERCOM2);
	QXK_ISR_EXIT();
}
#endif
#if defined(SERCOM5) && CONFIG_SERCOM5
	void SERCOM5_Handler(void){
		QXK_ISR_ENTRY();
		sercom_handler(SERCOM5);
		QXK_ISR_EXIT();
	}
#endif
};

/*
QState AOSERCOM::stateName(AOSERCOM * const me, QEvt const * const e) {
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
		
		default: {
			status = Q_SUPER(&AOSERCOM::Root);
			break;
		}
	}
	return status;
}*/
