/*
 * bsp_dma.h
 *
 *  Created on: Apr 13, 2018
 *      Author: deanm
 */

#ifndef BSP_BSP_DMA_H_
#define BSP_BSP_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

enum dma_transfer_trigger_action{
    DMA_TRIGGER_ACTON_BLOCK       = DMAC_CHCTRLB_TRIGACT_BLOCK_Val,
    DMA_TRIGGER_ACTON_BEAT        = DMAC_CHCTRLB_TRIGACT_BEAT_Val,
    DMA_TRIGGER_ACTON_TRANSACTION = DMAC_CHCTRLB_TRIGACT_TRANSACTION_Val,
};

enum dma_callback_type {
    // First item here is for any transfer errors. A transfer error is
    // flagged if a bus error is detected during an AHB access or when
    // the DMAC fetches an invalid descriptor
    DMA_CALLBACK_TRANSFER_ERROR,
    DMA_CALLBACK_TRANSFER_DONE,
    DMA_CALLBACK_CHANNEL_SUSPEND,
    DMA_CALLBACK_N, // Number of available callbacks
};

enum dma_beat_size {
    DMA_BEAT_SIZE_BYTE = 0, // 8-bit
    DMA_BEAT_SIZE_HWORD,    // 16-bit
    DMA_BEAT_SIZE_WORD,     // 32-bit
};

enum dma_event_output_selection {
    DMA_EVENT_OUTPUT_DISABLE = 0, // Disable event generation
    DMA_EVENT_OUTPUT_BLOCK,       // Event strobe when block xfer complete
    DMA_EVENT_OUTPUT_RESERVED,
    DMA_EVENT_OUTPUT_BEAT,        // Event strobe when beat xfer complete
};

enum dma_block_action {
    DMA_BLOCK_ACTION_NOACT = 0,
    // Channel in normal operation and sets transfer complete interrupt
    // flag after block transfer
    DMA_BLOCK_ACTION_INT,
    // Trigger channel suspend after block transfer and sets channel
    // suspend interrupt flag once the channel is suspended
    DMA_BLOCK_ACTION_SUSPEND,
    // Sets transfer complete interrupt flag after a block transfer and
    // trigger channel suspend. The channel suspend interrupt flag will
    // be set once the channel is suspended.
    DMA_BLOCK_ACTION_BOTH,
};

// DMA step selection. This bit determines whether the step size setting
// is applied to source or destination address.
enum dma_step_selection {
    DMA_STEPSEL_DST = 0,
    DMA_STEPSEL_SRC,
};

// Address increment step size. These bits select the address increment step
// size. The setting apply to source or destination address, depending on
// STEPSEL setting.
enum dma_address_increment_stepsize {
    DMA_ADDRESS_INCREMENT_STEP_SIZE_1 = 0, // beat size * 1
    DMA_ADDRESS_INCREMENT_STEP_SIZE_2,     // beat size * 2
    DMA_ADDRESS_INCREMENT_STEP_SIZE_4,     // beat size * 4
    DMA_ADDRESS_INCREMENT_STEP_SIZE_8,     // etc...
    DMA_ADDRESS_INCREMENT_STEP_SIZE_16,
    DMA_ADDRESS_INCREMENT_STEP_SIZE_32,
    DMA_ADDRESS_INCREMENT_STEP_SIZE_64,
    DMA_ADDRESS_INCREMENT_STEP_SIZE_128,
};

#ifdef __cplusplus
}
#endif

void dmac_init();
void dmac_alloc(uint8_t channel);

DmacDescriptor *dmac_set_descriptor(
  uint8_t channel,
  void           *src,
  void           *dst,
  uint32_t        count,
  uint8_t         size,
  bool            srcInc,
  bool            dstInc);

static inline void dmac_set_action(uint8_t channel, uint8_t action)
{
    DMAC->CHID.bit.ID         = channel;
    DMAC->CHCTRLB.bit.TRIGACT = action;
}

static inline void dmac_set_trigger(uint8_t channel, uint8_t trigger)
{
    DMAC->CHID.bit.ID         = channel;
    DMAC->CHCTRLB.bit.TRIGSRC = trigger;
}

static inline void dmac_start(uint8_t channel)
{
    DMAC->CHID.bit.ID    = channel;
    DMAC->CHCTRLA.bit.ENABLE = 1; // Enable the transfer channel
}

static inline void dmac_abort(uint8_t channel)
{
    DMAC->CHID.bit.ID = channel; // Select channel
    DMAC->CHCTRLA.reg = 0;       // Disable

    while(DMAC->CHCTRLA.bit.ENABLE);
}

bool dmac_is_active(uint8_t channel);

uint16_t dmac_get_transfer_count(uint8_t channel);

#endif /* BSP_BSP_DMA_H_ */
