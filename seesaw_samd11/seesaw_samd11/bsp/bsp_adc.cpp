#include "bsp_adc.h"

void adc_init()
{
	// Initialize Analog Controller
	// Setting clock
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_ADC_Val ) | // Generic Clock ADC
						GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
						GCLK_CLKCTRL_CLKEN ;
						
	while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains
	
	ADC->CTRLA.bit.SWRST = 1;
	
	while( ADC->STATUS.bit.SYNCBUSY == 1 || ADC->CTRLA.bit.SWRST == 1);

	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 |    // Divide Clock by 64.
					ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default

	ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length

	while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;   // No Negative input (Internal Ground)

	// Averaging (see datasheet table in AVGCTRL register description)
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |    // 1 sample only (no oversampling nor averaging)
						ADC_AVGCTRL_ADJRES(0x0ul);   // Adjusting result by 0

	ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA = 0.5* 3V3 = 1.65V
}

void adc_set_freerunning(bool mode)
{
	syncADC();
	ADC->CTRLA.bit.ENABLE = 0;
	
	syncADC();
	ADC->CTRLB.bit.FREERUN = mode;
	
	syncADC();
	ADC->CTRLA.bit.ENABLE = mode;
	
	syncADC();
}

void adc_set_inputscan(uint8_t channels)
{
	syncADC();
	ADC->INPUTCTRL.bit.INPUTSCAN = channels;
}

uint16_t adc_read(uint8_t channel)
{
	syncADC();
	ADC->INPUTCTRL.bit.MUXPOS = channel; // Selection for the positive ADC input

	// Control A
	/*
	* Bit 1 ENABLE: Enable
	*   0: The ADC is disabled.
	*   1: The ADC is enabled.
	* Due to synchronization, there is a delay from writing CTRLA.ENABLE until the peripheral is enabled/disabled. The
	* value written to CTRL.ENABLE will read back immediately and the Synchronization Busy bit in the Status register
	* (STATUS.SYNCBUSY) will be set. STATUS.SYNCBUSY will be cleared when the operation is complete.
	*
	* Before enabling the ADC, the asynchronous clock source must be selected and enabled, and the ADC reference must be
	* configured. The first conversion after the reference is changed must not be used.
	*/
	syncADC();
	ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC

	// Start conversion
	syncADC();
	ADC->SWTRIG.bit.START = 1;

	// Clear the Data Ready flag
	ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

	// Start conversion again, since The first conversion after the reference is changed must not be used.
	syncADC();
	ADC->SWTRIG.bit.START = 1;

	// Store the value
	while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
	uint16_t valueRead = ADC->RESULT.reg;

	syncADC();
	ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
	syncADC();
	
	return valueRead;
}