#include "bsp_timer.h"
#include "PinMap.h"
#include "SeesawConfig.h"

//TODO: this will probably change based on the use of the timer
void initTimerPWM( Tc *TCx )
{
	GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC1_TC2));
	while (GCLK->STATUS.bit.SYNCBUSY == 1);
	
	// Disable TCx
	TCx->COUNT8.CTRLA.bit.ENABLE = 0;
	syncTC_8(TCx);
	// Set Timer counter Mode to 8 bits, normal PWM, prescaler 1/256
	TCx->COUNT8.CTRLA.reg |= TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_PRESCALER_DIV64;
	syncTC_8(TCx);
	
	// Set the initial values
	TCx->COUNT8.CC[0].reg = 0x00;
	TCx->COUNT8.CC[1].reg = 0x00;
	syncTC_8(TCx);
	
	// Set PER to maximum counter value (resolution : 0xFF)
	TCx->COUNT8.PER.reg = 0xFF;
	syncTC_8(TCx);
	
	enableTimer(TCx);
}

#if CONFIG_TIMER
void PWMWrite( uint8_t pwm, uint8_t value)
{
	_PWM p = g_pwms[pwm];
	p.tc->COUNT8.CC[p.wo].reg = value;
	syncTC_8(p.tc);
}
#endif