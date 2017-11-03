#include "bsp_timer.h"
#include "PinMap.h"
#include "SeesawConfig.h"

//TODO: this will probably change based on the use of the timer
void initTimerPWM( Tc *TCx )
{
	#ifdef GCLK_CLKCTRL_ID_TC1_TC2
	GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC1_TC2));
	#else
	//TODO: for samd21
	//GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC3_TC4));
	#endif
	while (GCLK->STATUS.bit.SYNCBUSY == 1);
	
	// Disable TCx
	TCx->COUNT16.CTRLA.bit.ENABLE = 0;
	syncTC_16(TCx);
	// Set Timer counter Mode to 16 bits, normal PWM, prescaler 1/256
	TCx->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_PRESCALER_DIV16;
	syncTC_16(TCx);
	
	// Set the initial values
	TCx->COUNT16.CC[0].reg = 0x00;
	TCx->COUNT16.CC[1].reg = 0x00;
	syncTC_16(TCx);
	
	// Set PER to maximum counter value (resolution : 0xFF)
	// in 16 bit mode this is fixed
	//TCx->COUNT16.PER.reg = 0xFF;
	//syncTC_16(TCx);
	
	enableTimer(TCx);
}

#if CONFIG_TIMER
void PWMWrite( uint8_t pwm, uint16_t value)
{
	_PWM p = g_pwms[pwm];
	p.tc->COUNT16.CC[p.wo].reg = value;
	syncTC_16(p.tc);
}

void setFreq( uint8_t pwm, uint8_t freq )
{
	_PWM p = g_pwms[pwm];
	
	p.tc->COUNT16.CTRLA.bit.ENABLE = 0;
	syncTC_16(p.tc);
	p.tc->COUNT16.CTRLA.bit.PRESCALER = freq;
	enableTimer(p.tc);
}
#endif