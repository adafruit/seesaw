#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "sam.h"
#include "SeesawConfig.h"

#define PORTA 0
#define PORTB 1
#define PORTC 2
#define NO_PORT 3

/* TODO
*
PA27 should be the default 'activity LED'
PA16 and PA17 should be address select
so whatever i2c address we pick, if PA16 is grounded it adds +1 and PA17 ground adds +2
*/

#define PIN_ACTIVITY_LED 27

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