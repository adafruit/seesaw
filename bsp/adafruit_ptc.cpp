/*
 * FreeTouch, a QTouch-compatible library - tested on ATSAMD21 only!
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Limor 'ladyada' Fried for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "adafruit_ptc.h"
#include "pinmux.h"

static void sync_config(Ptc const* module_inst) {
    while (module_inst->CONTROLB.bit.SYNCFLAG) ;
}

void init_clocks_ptc(){

#if defined(SAMD21)
    /* Setup and enable generic clock source for PTC module.
      struct system_gclk_chan_config gclk_chan_conf;
      system_gclk_chan_get_config_defaults(&gclk_chan_conf);
   */

   uint8_t channel = PTC_GCLK_ID;
   uint8_t source_generator = 3;

   // original line: system_gclk_chan_set_config(PTC_GCLK_ID, &gclk_chan_conf);
   uint32_t new_clkctrl_config = (channel << GCLK_CLKCTRL_ID_Pos);  // from gclk.c

   // original line: gclk_chan_conf.source_generator = GCLK_GENERATOR_1;
   /* Select the desired generic clock generator */
   new_clkctrl_config |= source_generator << GCLK_CLKCTRL_GEN_Pos;  // from gclk.c

   /* Disable generic clock channel */
   // original line: system_gclk_chan_disable(channel);
   //noInterrupts();

   /* Select the requested generator channel */
   *((uint8_t*)&GCLK->CLKCTRL.reg) = channel;

   /* Sanity check WRTLOCK */
   //Assert(!GCLK->CLKCTRL.bit.WRTLOCK);

   /* Switch to known-working source so that the channel can be disabled */
   uint32_t prev_gen_id = GCLK->CLKCTRL.bit.GEN;
   GCLK->CLKCTRL.bit.GEN = 0;

   /* Disable the generic clock */
   GCLK->CLKCTRL.reg &= ~GCLK_CLKCTRL_CLKEN;
   while (GCLK->CLKCTRL.reg & GCLK_CLKCTRL_CLKEN) {
     /* Wait for clock to become disabled */
   }

   /* Restore previous configured clock generator */
   GCLK->CLKCTRL.bit.GEN = prev_gen_id;

   //system_interrupt_leave_critical_section();
   //interrupts();

   /* Write the new configuration */
   GCLK->CLKCTRL.reg = new_clkctrl_config;

   // original line: system_gclk_chan_enable(PTC_GCLK_ID);
   *((uint8_t*)&GCLK->CLKCTRL.reg) = channel;
   GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;    /* Enable the generic clock */


   // original line: system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_PTC);
   PM->APBCMASK.reg |= PM_APBCMASK_PTC;
#else

    // Sets up the clocks needed for the PTC module. The clock setup is described
    // in the touch.c implementation in the ASF examples I've replaced the ASF
    // calls with their low-level equivilents. Funny enough, the Generic Clock
    // Generator 3 is not actually used by arduinio core although it is set up in
    // various places.


    const unsigned int PTC_CLOCK_GCGID = 3;

    // Added for PTC - rough hack - JAG

    SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_2_Val ;  // recent versions of CMSIS from Atmel changed the prescaler defines
    SYSCTRL->OSC8M.bit.ONDEMAND = 1 ;
    //  SYSCTRL->OSC8M.bit.RUNINSTANDBY = 1 ;

    /* Put OSC8M as source for Generic Clock Generator 3 */
    GCLK->GENDIV.reg = GCLK_GENDIV_ID( PTC_CLOCK_GCGID) ; // Generic Clock Generator 3
    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 );

    // This magic actually does the assignment
    GCLK->GENCTRL.reg =
    ( GCLK_GENCTRL_ID( PTC_CLOCK_GCGID ) |
        GCLK_GENCTRL_SRC_OSC8M
        | GCLK_GENCTRL_GENEN
    );
    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 );

    // Original ASF code

    // const int GCLK_GENERATOR_3 = 3;
    // struct system_gclk_chan_config gclk_chan_conf;

    // system_gclk_chan_get_config_defaults(&gclk_chan_conf);
    // gclk_chan_conf->source_generator = GCLK_GENERATOR_0;
    // equivilent to:
    //  gclk_chan_conf.source_generator = GCLK_GENERATOR_3;

    //  system_gclk_chan_set_config(PTC_GCLK_ID, &gclk_chan_conf);
    // equivalent to
    uint32_t new_clkctrl_config = (PTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos);
    new_clkctrl_config |= PTC_CLOCK_GCGID << GCLK_CLKCTRL_GEN_Pos;

    //  system_gclk_chan_disable(channel);
    // this is the "disable" function
    /* Select the requested generator channel */
    *((uint8_t*)&GCLK->CLKCTRL.reg) = PTC_CLOCK_GCGID;

    /* Switch to known-working source so that the channel can be disabled */
    uint32_t prev_gen_id = GCLK->CLKCTRL.bit.GEN;
    GCLK->CLKCTRL.bit.GEN = 0;

    /* Disable the generic clock */
    GCLK->CLKCTRL.reg &= ~GCLK_CLKCTRL_CLKEN;
    while (GCLK->CLKCTRL.reg & GCLK_CLKCTRL_CLKEN) {
    /* Wait for clock to become disabled */
    }

    /* Restore previous configured clock generator */
    GCLK->CLKCTRL.bit.GEN = prev_gen_id;

    /* Write the new configuration */
    GCLK->CLKCTRL.reg = new_clkctrl_config;

    //  system_gclk_chan_enable(PTC_GCLK_ID);
    // equivlent to:
    *((uint8_t*)&GCLK->CLKCTRL.reg) = PTC_GCLK_ID;
    GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;


    // system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_PTC);
    // equivilant is:
    PM->APBCMASK.reg |= PM_APBCMASK_PTC;

#endif
}

uint16_t touch_measure(struct adafruit_ptc_config *config) {
  uint16_t m;

  m = touch_measureRaw(config);
  if (m == -1) return -1;

  // normalize the signal
  switch (config->oversample) {
    case OVERSAMPLE_1:   return m;
    case OVERSAMPLE_2:   return m/2;
    case OVERSAMPLE_4:   return m/4;
    case OVERSAMPLE_8:   return m/8;
    case OVERSAMPLE_16:  return m/16;
    case OVERSAMPLE_32:  return m/32;
    case OVERSAMPLE_64:  return m/64;
  }

  return -1; // shouldn't reach here but fail if we do!
}

uint16_t touch_measureRaw(struct adafruit_ptc_config *config) {
    adafruit_ptc_start_conversion((Ptc*)PTC, config);

    while (!adafruit_ptc_is_conversion_finished((Ptc*)PTC));

    return adafruit_ptc_get_conversion_result((Ptc*)PTC);
}

void adafruit_ptc_get_config_default(struct adafruit_ptc_config *config) {
    config->pin = 0xff;
    config->yline = -1;
    config->oversample = OVERSAMPLE_4;
    config->seriesres = RESISTOR_0;
    config->freqhop = FREQ_MODE_NONE;
    config->compcap = 0x2000;
    config->intcap = 0x3F;
}

void adafruit_ptc_init(Ptc* module_inst, struct adafruit_ptc_config const* config) {
    struct system_pinmux_config pinmux_config;
    system_pinmux_get_config_defaults(&pinmux_config);
    pinmux_config.mux_position = 0x1;
    pinmux_config.input_pull = SYSTEM_PINMUX_PIN_PULL_NONE;
    system_pinmux_pin_set_config(config->pin, &pinmux_config);

    sync_config(module_inst);
    module_inst->CONTROLA.bit.ENABLE = 0;
    sync_config(module_inst);

    module_inst->UNK4C04.reg &= 0xF7; //MEMORY[0x42004C04] &= 0xF7u;
    module_inst->UNK4C04.reg &= 0xFB; //MEMORY[0x42004C04] &= 0xFBu;
    module_inst->UNK4C04.reg &= 0xFC; //MEMORY[0x42004C04] &= 0xFCu;
    sync_config(module_inst);
    module_inst->FREQCONTROL.reg &= 0x9F;       //MEMORY[0x42004C0C] &= 0x9Fu;
    sync_config(module_inst);
    module_inst->FREQCONTROL.reg &= 0xEF;       //MEMORY[0x42004C0C] &= 0xEFu;
    sync_config(module_inst);
    module_inst->FREQCONTROL.bit.SAMPLEDELAY = 0; //MEMORY[0x42004C0C] &= 0xF0u;
    module_inst->CONTROLC.bit.INIT = 1;         //MEMORY[0x42004C05] |= 1u;
    module_inst->CONTROLA.bit.RUNINSTANDBY = 1; //MEMORY[0x42004C00] |= 4u;
    sync_config(module_inst);
    module_inst->INTDISABLE.bit.WCO = 1;
    sync_config(module_inst);
    module_inst->INTDISABLE.bit.EOC = 1;
    sync_config(module_inst);

    // enable the sensor, only done once per line
    if (config->yline < 8) {
        sync_config(module_inst);
        module_inst->YENABLEL.reg |= 1 << config->yline;
        sync_config(module_inst);
    } else if (config->yline < 16) {
        module_inst->YENABLEH.reg |= 1 << (config->yline - 8);
    }

    sync_config(module_inst);
    module_inst->CONTROLA.bit.ENABLE = 1;
    sync_config(module_inst);
}

void adafruit_ptc_start_conversion(Ptc* module_inst, struct adafruit_ptc_config const* config) {
    module_inst->CONTROLA.bit.RUNINSTANDBY = 1;
    sync_config(module_inst);
    module_inst->CONTROLA.bit.ENABLE = 1;
    sync_config(module_inst);
    module_inst->INTDISABLE.bit.WCO = 1;
    sync_config(module_inst);
    module_inst->INTFLAGS.bit.WCO = 1;
    sync_config(module_inst);
    module_inst->INTFLAGS.bit.EOC = 1;
    sync_config(module_inst);

    // set up pin!
    sync_config(module_inst);
    if (config->yline < 8) {
        module_inst->YSELECTL.reg = 1 << config->yline;
    } else {
        module_inst->YSELECTL.reg = 0;
    }

    if (config->yline > 7) {
        module_inst->YSELECTH.reg = 1 << (config->yline - 8);
    } else {
        module_inst->YSELECTH.reg = 0;
    }

    sync_config(module_inst);
    // set up sense resistor
    module_inst->SERRES.bit.RESISTOR = config->seriesres;
    sync_config(module_inst);
    // set up prescalar
    module_inst->CONVCONTROL.bit.ADCACCUM = config->oversample;
    sync_config(module_inst);
    // set up freq hopping
    if (config->freqhop == FREQ_MODE_NONE) {
        module_inst->FREQCONTROL.bit.FREQSPREADEN = 0;
        module_inst->FREQCONTROL.bit.SAMPLEDELAY = 0;
    } else {
        module_inst->FREQCONTROL.bit.FREQSPREADEN = 1;
        module_inst->FREQCONTROL.bit.SAMPLEDELAY = config->hops;
    }
    // set up compensation cap + int (?) cap
    sync_config(module_inst);
    module_inst->COMPCAPL.bit.VALUE = config->compcap & 0xFF;
    module_inst->COMPCAPH.bit.VALUE = (config->compcap>>8) & 0x3F;
    sync_config(module_inst);
    module_inst->INTCAP.bit.VALUE = config->intcap & 0x3F;
    sync_config(module_inst);

    module_inst->BURSTMODE.reg = 0xA4;
    sync_config(module_inst);

    module_inst->CONVCONTROL.bit.CONVERT = 1;
    sync_config(module_inst);
}

bool adafruit_ptc_is_conversion_finished(Ptc* module_inst) {
    return module_inst->CONVCONTROL.bit.CONVERT == 0;
}

uint16_t adafruit_ptc_get_conversion_result(Ptc* module_inst) {
    sync_config(module_inst);
    return module_inst->RESULT.reg;
}
