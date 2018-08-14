/*
 * bsp_dma.cpp
 *
 *  Created on: Apr 13, 2018
 *      Author: deanm
 */

#include "sam.h"
#include <string.h>

#include "bsp_dma.h"

__attribute__((__aligned__(16))) static DmacDescriptor // 128 bit alignment
  _descriptor[DMAC_CH_NUM] SECTION_DMAC_DESCRIPTOR,
  _writeback[DMAC_CH_NUM]  SECTION_DMAC_DESCRIPTOR;

void dmac_init()
{
    PM->AHBMASK.bit.DMAC_       = 1; // Initialize DMA clocks
    PM->APBBMASK.bit.DMAC_      = 1;

    NVIC_DisableIRQ(DMAC_IRQn);
    DMAC->CTRL.bit.DMAENABLE    = 0; // Disable DMA controller
    while(DMAC->CTRL.bit.DMAENABLE);
    DMAC->CTRL.bit.SWRST        = 1; // Perform software reset

    // Initialize descriptor list addresses
    DMAC->BASEADDR.bit.BASEADDR = (uint32_t)_descriptor;
    DMAC->WRBADDR.bit.WRBADDR   = (uint32_t)_writeback;
    memset(_descriptor, 0, sizeof(_descriptor));
    memset(_writeback , 0, sizeof(_writeback));

    // Re-enable DMA controller with all priority levels
    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);

    // Enable DMA interrupt at lowest priority
    NVIC_EnableIRQ(DMAC_IRQn);
    NVIC_SetPriority(DMAC_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
}

void dmac_alloc(uint8_t channel){
    // Reset the allocated channel
    DMAC->CHID.bit.ID         = channel;
    DMAC->CHCTRLA.bit.ENABLE  = 0;
    DMAC->CHCTRLA.bit.SWRST   = 1;

    // Clear software trigger
    DMAC->SWTRIGCTRL.reg     &= ~(1 << channel);

    // Configure default behaviors
    DMAC->CHCTRLB.bit.LVL     = 0;
    DMAC->CHCTRLB.bit.TRIGSRC = 0;
    DMAC->CHCTRLB.bit.TRIGACT = DMAC_CHCTRLB_TRIGACT_TRANSACTION_Val;
}

DmacDescriptor *dmac_set_descriptor(
  uint8_t channel,
  void           *src,
  void           *dst,
  uint32_t        count,
  uint8_t         size,
  bool            srcInc,
  bool            dstInc) {

    DmacDescriptor *desc;


    desc = &_descriptor[channel];

    uint8_t bytesPerBeat; // Beat transfer size IN BYTES
    switch(size) {
       default:                  bytesPerBeat = 1; break;
       case DMA_BEAT_SIZE_HWORD: bytesPerBeat = 2; break;
       case DMA_BEAT_SIZE_WORD:  bytesPerBeat = 4; break;
    }

    desc->BTCTRL.bit.VALID    = true;
    desc->BTCTRL.bit.EVOSEL   = DMA_EVENT_OUTPUT_DISABLE;
    desc->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_NOACT;
    desc->BTCTRL.bit.BEATSIZE = size;
    desc->BTCTRL.bit.SRCINC   = srcInc;
    desc->BTCTRL.bit.DSTINC   = dstInc;
    desc->BTCTRL.bit.STEPSEL  = DMA_STEPSEL_DST;
    desc->BTCTRL.bit.STEPSIZE = DMA_ADDRESS_INCREMENT_STEP_SIZE_1;
    desc->BTCNT.reg           = count;
    desc->SRCADDR.reg         = (uint32_t)src;
    if(srcInc) desc->SRCADDR.reg += bytesPerBeat * count;
    desc->DSTADDR.reg         = (uint32_t)dst;
    if(dstInc) desc->DSTADDR.reg += bytesPerBeat * count;
        desc->DESCADDR.reg = 0;

    return desc;
}

uint16_t dmac_get_transfer_count(uint8_t channel)
{
    return _descriptor[channel].BTCNT.reg - _writeback[channel].BTCNT.reg;
}

bool dmac_is_active(uint8_t channel){
    return _writeback[channel].BTCTRL.bit.VALID;
}

