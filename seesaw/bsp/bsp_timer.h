#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "sam.h"

inline void syncTC_8(Tc* TCx) {
	while (TCx->COUNT8.STATUS.bit.SYNCBUSY);
}

inline void syncTC_16(Tc* TCx) {
	while (TCx->COUNT16.STATUS.bit.SYNCBUSY);
}

inline void enableTimer( Tc *TCx ){
	TCx->COUNT16.CTRLA.bit.ENABLE = 1;
	syncTC_16(TCx);
}

void initTimerPWM( Tc *TCx );

void PWMWrite( uint8_t pwm, uint16_t value);
void setFreq( uint8_t pwm, uint8_t freq);

#endif