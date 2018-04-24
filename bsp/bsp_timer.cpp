#include "bsp_timer.h"
#include "PinMap.h"
#include "SeesawConfig.h"

#if CONFIG_TIMER
_PWM g_pwms[]=
{
#ifdef USE_TCC_TIMERS
    { CONFIG_TIMER_PWM_OUT0_TC, CONFIG_TIMER_PWM_OUT0_TCC, CONFIG_TIMER_PWM_OUT0_WO, CONFIG_TIMER_PWM_OUT0_PIN },
    { CONFIG_TIMER_PWM_OUT1_TC, CONFIG_TIMER_PWM_OUT1_TCC, CONFIG_TIMER_PWM_OUT1_WO, CONFIG_TIMER_PWM_OUT1_PIN },
    { CONFIG_TIMER_PWM_OUT2_TC, CONFIG_TIMER_PWM_OUT2_TCC, CONFIG_TIMER_PWM_OUT2_WO, CONFIG_TIMER_PWM_OUT2_PIN },
    { CONFIG_TIMER_PWM_OUT3_TC, CONFIG_TIMER_PWM_OUT3_TCC, CONFIG_TIMER_PWM_OUT3_WO, CONFIG_TIMER_PWM_OUT3_PIN },
    { CONFIG_TIMER_PWM_OUT4_TC, CONFIG_TIMER_PWM_OUT4_TCC, CONFIG_TIMER_PWM_OUT4_WO, CONFIG_TIMER_PWM_OUT4_PIN },
    { CONFIG_TIMER_PWM_OUT5_TC, CONFIG_TIMER_PWM_OUT5_TCC, CONFIG_TIMER_PWM_OUT5_WO, CONFIG_TIMER_PWM_OUT5_PIN },
    { CONFIG_TIMER_PWM_OUT6_TC, CONFIG_TIMER_PWM_OUT6_TCC, CONFIG_TIMER_PWM_OUT6_WO, CONFIG_TIMER_PWM_OUT6_PIN },
    { CONFIG_TIMER_PWM_OUT7_TC, CONFIG_TIMER_PWM_OUT7_TCC, CONFIG_TIMER_PWM_OUT7_WO, CONFIG_TIMER_PWM_OUT7_PIN },
    { CONFIG_TIMER_PWM_OUT8_TC, CONFIG_TIMER_PWM_OUT8_TCC, CONFIG_TIMER_PWM_OUT8_WO, CONFIG_TIMER_PWM_OUT8_PIN },
    { CONFIG_TIMER_PWM_OUT9_TC, CONFIG_TIMER_PWM_OUT6_TCC, CONFIG_TIMER_PWM_OUT9_WO, CONFIG_TIMER_PWM_OUT9_PIN },
    { CONFIG_TIMER_PWM_OUT10_TC, CONFIG_TIMER_PWM_OUT10_TCC, CONFIG_TIMER_PWM_OUT10_WO, CONFIG_TIMER_PWM_OUT10_PIN },
    { CONFIG_TIMER_PWM_OUT11_TC, CONFIG_TIMER_PWM_OUT11_TCC, CONFIG_TIMER_PWM_OUT11_WO, CONFIG_TIMER_PWM_OUT11_PIN },

#else
    { CONFIG_TIMER_PWM_OUT0_TC, CONFIG_TIMER_PWM_OUT0_WO, CONFIG_TIMER_PWM_OUT0_PIN },
    { CONFIG_TIMER_PWM_OUT1_TC, CONFIG_TIMER_PWM_OUT1_WO, CONFIG_TIMER_PWM_OUT1_PIN },
    { CONFIG_TIMER_PWM_OUT2_TC, CONFIG_TIMER_PWM_OUT2_WO, CONFIG_TIMER_PWM_OUT2_PIN },
    { CONFIG_TIMER_PWM_OUT3_TC, CONFIG_TIMER_PWM_OUT3_WO, CONFIG_TIMER_PWM_OUT3_PIN },
#endif //USE_TCC_TIMERS
};
#endif

//TODO: this will probably change based on the use of the timer
void initTimerPWM( Tc *TCx )
{
    initTimerClocks();
	
#ifdef GCLK_CLKCTRL_ID_TC4_TC5
	GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC4_TC5_Val));
	while (GCLK->STATUS.bit.SYNCBUSY == 1);
#endif

	// Disable TCx
	TCx->COUNT16.CTRLA.bit.ENABLE = 0;
	syncTC_16(TCx);
	// Set Timer counter Mode to 16 bits, normal PWM, prescaler 1/16
	TCx->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_PRESCALER_DIV16;
	syncTC_16(TCx);
	
	// Set the initial values
	TCx->COUNT16.CC[0].reg = 0x00;
	TCx->COUNT16.CC[1].reg = 0x00;
	syncTC_16(TCx);
	
	enableTimer(TCx);
}

#ifdef USE_TCC_TIMERS
void initTimerPWM( Tcc *TCCx )
{
    initTimerClocks();
    // Disable TCCx
      TCCx->CTRLA.bit.ENABLE = 0;
      syncTCC(TCCx);
      // Set prescaler to 1/16
      TCCx->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV16;
      syncTCC(TCCx);
      // Set TCx as normal PWM
      TCCx->WAVE.reg |= TCC_WAVE_WAVEGEN_NPWM;
      syncTCC(TCCx);

      // Set the initial value
      TCCx->CC[0].reg = (uint32_t) 0;
      while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);

      TCCx->PER.reg = (48000000/(16 * 2000)) - 1; //2khz default
      syncTCC(TCCx);

      enableTimer(TCCx);
}
#endif

#if CONFIG_TIMER
void PWMWrite( uint8_t pwm, uint16_t value)
{
	_PWM p = g_pwms[pwm];
#ifdef USE_TCC_TIMERS
    if(p.tc != NOT_ON_TC){
#endif
        p.tc->COUNT16.CC[p.wo].reg = value;
        syncTC_16(p.tc);
#ifdef USE_TCC_TIMERS
    }
    else{
        uint32_t top = p.tcc->PER.reg;
        uint32_t val = (uint32_t)value * top / 65535UL; //map to current top value
        while (p.tcc->SYNCBUSY.bit.CTRLB);
        while (p.tcc->SYNCBUSY.bit.CC0 || p.tcc->SYNCBUSY.bit.CC1);
        p.tcc->CCB[p.wo].reg = val;
        while (p.tcc->SYNCBUSY.bit.CC0 || p.tcc->SYNCBUSY.bit.CC1);
        p.tcc->CTRLBCLR.bit.LUPD = 1;
        while (p.tcc->SYNCBUSY.bit.CTRLB);
    }
#endif
}

void setFreq( uint8_t pwm, uint16_t freq )
{
    _PWM p = g_pwms[pwm];
#ifdef USE_TCC_TIMERS
    if(p.tc != NOT_ON_TC){
#endif
        uint8_t prescale = TC_CTRLA_PRESCALER_DIV256_Val;
        if( freq > 500) prescale = TC_CTRLA_PRESCALER_DIV1_Val;
        else if( freq > 250 ) prescale = TC_CTRLA_PRESCALER_DIV2_Val;
        else if( freq > 140 ) prescale = TC_CTRLA_PRESCALER_DIV4_Val;
        else if( freq > 75 ) prescale = TC_CTRLA_PRESCALER_DIV8_Val;
        else if( freq > 25 ) prescale = TC_CTRLA_PRESCALER_DIV16_Val;
        else if( freq > 7 ) prescale = TC_CTRLA_PRESCALER_DIV64_Val;

        p.tc->COUNT16.CTRLA.bit.ENABLE = 0;
        syncTC_16(p.tc);
        p.tc->COUNT16.CTRLA.bit.PRESCALER = prescale;
        enableTimer(p.tc);
#ifdef USE_TCC_TIMERS
    }
    else{
        //fpwm = 48000000/prescale * (PER + 1)
        syncTCC(p.tcc);
        p.tcc->PER.reg = (48000000/(16 * freq)) - 1;
        syncTCC(p.tcc);
    }
#endif
}
#endif
