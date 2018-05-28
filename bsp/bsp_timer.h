#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "sam.h"
#include "SeesawConfig.h"

inline void syncTC_8(Tc* TCx) {
	while (TCx->COUNT8.STATUS.bit.SYNCBUSY);
}

inline void syncTC_16(Tc* TCx) {
	while (TCx->COUNT16.STATUS.bit.SYNCBUSY);
}

#ifdef USE_TCC_TIMERS
inline void syncTCC(Tcc* TCCx) {
    while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK);
}
#endif

inline void enableTimer( Tc *TCx ){
	TCx->COUNT16.CTRLA.bit.ENABLE = 1;
	syncTC_16(TCx);
}

#ifdef USE_TCC_TIMERS
inline void enableTimer( Tcc *TCCx ){
    TCCx->CTRLA.bit.ENABLE = 1;
    syncTCC(TCCx);
}
#endif

inline void initTimerClocks(){
#ifdef GCLK_CLKCTRL_ID_TC1_TC2
    GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC1_TC2_Val));
#else
    GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC0_TCC1_Val));
    while (GCLK->STATUS.bit.SYNCBUSY == 1);
    GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3_Val));
#endif
    while (GCLK->STATUS.bit.SYNCBUSY == 1);
}

void initTimerPWM( Tc *TCx );

#ifdef USE_TCC_TIMERS
void initTimerPWM( Tcc *TCCx );
#endif

void PWMWrite( uint8_t pwm, uint16_t value);
void setFreq( uint8_t pwm, uint16_t freq);

#endif
