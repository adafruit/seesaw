#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "sam.h"
#include "SeesawConfig.h"

#define PORTA 0
#define PORTB 1
#define PORTC 2
#define NO_PORT 3

#define PIN_ACTIVITY_LED 27
#define PIN_ADDR_0 16
#define PIN_ADDR_1 17

#if CONFIG_TIMER
	typedef struct _PWM
	{
		Tc*				tc ;
		uint8_t			wo ;
		uint8_t			pin;
	} _PWM ;

	const _PWM g_pwms[]=
	{
		{ CONFIG_TIMER_PWM_OUT0_TC, CONFIG_TIMER_PWM_OUT0_WO, CONFIG_TIMER_PWM_OUT0_PIN },
		{ CONFIG_TIMER_PWM_OUT1_TC, CONFIG_TIMER_PWM_OUT1_WO, CONFIG_TIMER_PWM_OUT1_PIN },
		{ CONFIG_TIMER_PWM_OUT2_TC, CONFIG_TIMER_PWM_OUT2_WO, CONFIG_TIMER_PWM_OUT2_PIN },
		{ CONFIG_TIMER_PWM_OUT3_TC, CONFIG_TIMER_PWM_OUT3_WO, CONFIG_TIMER_PWM_OUT3_PIN },
	};
#endif

#endif