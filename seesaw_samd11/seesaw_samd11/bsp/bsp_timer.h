#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "sam.h"

inline void syncTC_8(Tc* TCx) {
	while (TCx->COUNT8.STATUS.bit.SYNCBUSY);
}

inline void enableTimer( Tc *TCx ){
	TCx->COUNT8.CTRLA.bit.ENABLE = 1;
	syncTC_8(TCx);
}

void initTimerPWM( Tc *TCx );

void PWMWrite( uint8_t pwm, uint8_t value);

#endif