#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "sam.h"
#include "SeesawConfig.h"

#define PORTA 0
#define PORTB 1
#define PORTC 2

//TODO: this will change once we figure out how we wanna number the pins
/*
 * Pins descriptions
 */

/* Types used for the table below */
typedef struct _PinDescription
{
	uint32_t       ulPort ;
	uint32_t        ulPin ;
} PinDescription ;

#if defined(__SAMD21G18A__)

	const PinDescription g_APinDescription[]=
	{
	  { PORTB, 23 },
	  { PORTB, 22 },
	  { PORTA, 17 },
	  { PORTA, 2 },
	  { PORTB, 8 },
	  { PORTA, 22 }, //SDA
	  { PORTA, 23 }, //SCL
	  { PORTA, 14 }, //pin 2
	} ;

#else

	const PinDescription g_APinDescription[]=
	{
		{ PORTA, 11 },
		{ PORTA, 10 },
		{ PORTA, 17 },
		{ PORTA, 2 },
		{ PORTB, 8 },
		{ PORTA, 22 }, //SDA
		{ PORTA, 23 }, //SCL
		{ PORTA, 14 }, //pin 2
		{ PORTA, 4	}, //TC1 WO0
		{ PORTA, 5	}, //TC1 WO1
		{ PORTA, 6	}, //TC2 WO0
		{ PORTA, 7	}, //TC2 WO1
	} ;

#endif

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