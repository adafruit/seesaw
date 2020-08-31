#include "qpcpp.h"
#include "qp_extras.h"
#include "bsp_gpio.h"
#include "PinMap.h"

#include "DPIassist.h"

#if CONFIG_DPIASSIST

static int _clk, _dat, _cs, _rst;

const uint8_t init61408[] = {
  0x01, 0,  // Soft reset
  0xFF, 125, // delay 125
  0x11, 0,  // Sleep Mode Off
  0xFF, 125, // delay 125
  0xB0, 1, 0x04,  // MCAP(Let Command Protect Off)
  0xB3, 2, 0x12, 0x00, // EPF[1:0]=00¡ADM[1:0]=00(OSC)¡AWEMODE=10;
  0xB6, 2, 0x52, 0x83,
  0xB7, 4, 0x00, 0x00, 0x11, 0x25,
  0xBD, 1, 0x00,
  0xC0, 2, 0x02, 0x87,
  0xC1, 15, 0x42, 0x31, 0x00, 0x21, 0x21, 0x32, 0x12, 0x28, 0x4A, 0x1E, 0xA5, 0x0F, 0x58, 0x21, 0x01,
  0xC2, 6, 0x28, 0x06, 0x06, 0x01, 0x03, 0x00,
  0xC6, 2, 0x00, 0x00,
  0xC8, 24, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05,
  0xC9, 24, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05,
  0xCA, 24, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05, 0x00, 0x15, 0x22, 0x2C, 0x39, 0x54, 0x34, 0x21, 0x16, 0x11, 0x08, 0x05,
  0xD0, 16, 0x29, 0x03, 0xCE, 0xA6, 0x00, 0x71, 0x20, 0x10, 0x01, 0x00, 0x01, 0x01, 0x00, 0x03, 0x01, 0x00,
  0xFD, 8, 0x00, 0x00, 0x70, 0x00, 0x32, 0x31, 0x34, 0x36,
  0xD3, 1, 0x33,
  0xD5, 2, 0x1A, 0x1A,
  0xD6, 1, 0xA8,
  0xDE, 2, 0x01, 0x52,
  0xE0, 4, 0x00, 0x00, 0x00, 0x00,
  0xE1, 6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFA, 1, 0x03,
  0xB0, 1, 0x03,
  0x2A, 4, 0x00, 0x00, 0x01, 0xDF,
  0x2B, 4, 0x00, 0x00, 0x03, 0x1F,
  0x36, 1, 0x40,   // Address mode
  0x3A, 1, 0x77,   // 24 bit color
  0x35, 1, 0x00,   // Tear ON
  0x29, 0,         // Display ON
  0xFF, 50,
  0x2C, 0,         // Write memory
  0xFF, 50,
  0,
};

volatile bool DPIASSIST_VSYNC_ACTIVE = false;
volatile uint32_t DPI_VSYNC_TIMERMS = 0xFFFFFFFF;
#define DPI_VSYNC_TIMERMS_TIMEOUT 100 // 100 ms

extern "C" {
	void EIC_Handler(void)
	{
      // EXTINT0
      if ( EIC->INTFLAG.bit.EXTINT0 == 1 )
        {
          REG_EIC_INTFLAG = EIC_INTENSET_EXTINT0; //Clear Interrupt
          DPI_VSYNC_TIMERMS = 0; // reset timer
        }
	}
};

void DPIassist_tick(void) {
  if (DPI_VSYNC_TIMERMS != 0xFFFFFFFF) {
    DPI_VSYNC_TIMERMS++;
  }
  bool oldstate = DPIASSIST_VSYNC_ACTIVE;
  DPIASSIST_VSYNC_ACTIVE = DPI_VSYNC_TIMERMS < DPI_VSYNC_TIMERMS_TIMEOUT;
  gpio_write(PORTA, CONFIG_DPIASSIST_LED, DPIASSIST_VSYNC_ACTIVE);
  if (!oldstate & DPIASSIST_VSYNC_ACTIVE) {
    // initialize screen
    DPIassist_sendInit();
  }
}


void DPIassist_sendInit(void) {
  uint8_t cmd, arg, numArgs;

  const uint8_t *addr = init61408;

  while ((cmd = *addr++) > 0) {
    numArgs = *addr++;
    if (cmd == 0xFF) {
      delay_ms(numArgs);
      continue;
    }
    DPIassist_transfer(cmd, false);
    for (int i=0; i<numArgs; i++) {
      arg = *addr++;
      DPIassist_transfer(arg, true);
    }
  }
}


void delay_ms(uint32_t d) {
  while (d--) {
    uint32_t m = 8000;
    while (m--) {
      asm("nop");
    }
  }
}

void delay_us(uint32_t d) {
  while (d--) {
    uint32_t m = 8;
    while (m--) {
      asm("nop");
    }
  }
}

void DPIassist_init(uint8_t clockpin, uint8_t datapin, uint8_t cspin, uint8_t resetpin) {
  _clk = clockpin;
  _dat = datapin;
  _cs = cspin;
  _rst = resetpin;

  // Set all to be outputs
  gpio_init(PORTA, _clk, 1);
  gpio_init(PORTA, _dat, 1);
  gpio_init(PORTA, _cs, 1);
  gpio_init(PORTA, _rst, 1);

  // CS starts high
  gpio_write(PORTA,  _cs, 1);

  // RST starts low
  gpio_write(PORTA,  _rst, 0);

  // set VSYNC to input with pulldown
  // input
  gpio_init(PORTA, CONFIG_DPIASSIST_VSYNCIRQ, 1);
  gpio_init(PORTA, CONFIG_DPIASSIST_LED, 1);

  gpio_write(PORTA, CONFIG_DPIASSIST_LED, 0);
  gpio_write(PORTA, CONFIG_DPIASSIST_VSYNCIRQ, 0);

  REG_PM_APBAMASK |= PM_APBAMASK_EIC; //Enable EIC Clock

  PORT->Group[0].PINCFG[CONFIG_DPIASSIST_VSYNCIRQ].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[CONFIG_DPIASSIST_VSYNCIRQ >> 2].bit.PMUXE = MUX_PA16A_EIC_EXTINT0; 
  EIC->INTENSET.reg = EIC_INTENSET_EXTINT0;
  EIC->CONFIG[0].bit.SENSE0 = EIC_CONFIG_SENSE0_FALL;
  EIC->CTRL.reg |= EIC_CTRL_ENABLE;
  NVIC_EnableIRQ(EIC_IRQn);
}


void DPIassist_reset(void) {
  // low
  gpio_write(PORTA,  _rst, 0);
  delay_ms(10);
  // high
  gpio_write(PORTA,  _rst, 1);
  delay_ms(150);
}


void DPIassist_transfer(uint8_t data, bool dc) {

  // software SPI
  uint16_t send = data;
  send |= dc << 8;
  
  gpio_write(PORTA, _clk, 1);
  gpio_write(PORTA, _cs, 0);
  delay_us(3);
  for (int b = 8; b >= 0; b--) {
    gpio_write(PORTA, _dat, (send >> b) & 0x1);
    delay_us(3);
    gpio_write(PORTA, _clk, 0);
    delay_us(3);
    gpio_write(PORTA, _clk, 1);
  }
  delay_us(3);
  gpio_write(PORTA, _cs, 1);
  return;
}

#endif
