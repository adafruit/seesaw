/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "sam.h"
#include "bsp_gpio.h"
#include "PinMap.h"
#include "SeesawConfig.h"

#define DAP_CONFIG_ENABLE_SWD
//#define DAP_CONFIG_ENABLE_JTAG

#define DAP_CONFIG_DEFAULT_PORT        DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK       1000000 // Hz

#define DAP_CONFIG_PACKET_SIZE         64
#define DAP_CONFIG_PACKET_COUNT        1

// Set the value to NULL if you want to disable a string
// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR          "Adafruit"
#define DAP_CONFIG_PRODUCT_STR         "Seesaw CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         "123456"
#define DAP_CONFIG_FW_VER_STR          "v0.1"
#define DAP_CONFIG_DEVICE_VENDOR_STR   NULL
#define DAP_CONFIG_DEVICE_NAME_STR     NULL

//#define DAP_CONFIG_RESET_TARGET_FN     target_specific_reset_function

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
#define DAP_CONFIG_DELAY_CONSTANT      4700

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin 
#define DAP_CONFIG_FAST_CLOCK          3600000 // Hz

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value)
{
	gpio_write(PORTA, CONFIG_DAP_SWCLK, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value)
{
  gpio_write(PORTA, CONFIG_DAP_SWDIO, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value)
{
  gpio_write(PORTA, CONFIG_DAP_TDO, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value)
{
  gpio_write(PORTA, CONFIG_DAP_nTRST, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value)
{
  gpio_write(PORTA, CONFIG_DAP_nRESET, value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_SWCLK)) > 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_SWDIO)) > 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_TDI)) > 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_TDO)) > 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_nTRST)) > 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void)
{
  return (gpio_read_bulk() & (1 << CONFIG_DAP_nRESET)) > 0;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void)
{
  PORT->Group[PORTA].OUTSET.reg = (1ul<<CONFIG_DAP_SWCLK);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void)
{
  PORT->Group[PORTA].OUTCLR.reg = (1ul<<CONFIG_DAP_SWCLK);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 0);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 1);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWCLK, 0);
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 0);
  gpio_init(PORTA, CONFIG_DAP_TDI, 0);
  gpio_init(PORTA, CONFIG_DAP_TDO, 0);
  gpio_init(PORTA, CONFIG_DAP_nTRST, 0);
  gpio_init(PORTA, CONFIG_DAP_nRESET, 0);
  
  gpio_pullenset_bulk(1ul << CONFIG_DAP_SWDIO);
  gpio_outset_bulk(PORTA, 1ul << CONFIG_DAP_SWDIO);
  
  PORT->Group[PORTA].OUTSET.reg = (1ul<<PIN_ACTIVITY_LED);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWCLK, 0);
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 0);
  gpio_init(PORTA, CONFIG_DAP_TDI, 0);
  gpio_init(PORTA, CONFIG_DAP_TDO, 0);
  gpio_init(PORTA, CONFIG_DAP_nTRST, 0);
  gpio_init(PORTA, CONFIG_DAP_nRESET, 0);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_SWDIO));

  gpio_init(PORTA, CONFIG_DAP_SWCLK, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_SWCLK));

  gpio_init(PORTA, CONFIG_DAP_nRESET, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_nRESET));

  gpio_init(PORTA, CONFIG_DAP_TDI, 0);
  gpio_init(PORTA, CONFIG_DAP_TDO, 0);
  gpio_init(PORTA, CONFIG_DAP_nTRST, 0);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void)
{
  gpio_init(PORTA, CONFIG_DAP_SWDIO, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_SWDIO));

  gpio_init(PORTA, CONFIG_DAP_SWCLK, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_SWCLK));

  gpio_init(PORTA, CONFIG_DAP_TDI, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_TDI));

  gpio_init(PORTA, CONFIG_DAP_TDO, 0);

  gpio_init(PORTA, CONFIG_DAP_nRESET, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_nRESET));

  gpio_init(PORTA, CONFIG_DAP_nTRST, 1);
  gpio_outset_bulk(PORTA, (1ul << CONFIG_DAP_nTRST));
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state)
{
  if (0 == index)
    gpio_write(PORTA, PIN_ACTIVITY_LED, !state);
}

#endif // _DAP_CONFIG_H_

