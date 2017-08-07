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

//TODO: this will change once we figure out how we wanna number the pins
/*
 * Pins descriptions
 */

typedef enum
{
	EXTERNAL_INT_0 = 0,
	EXTERNAL_INT_1,
	EXTERNAL_INT_2,
	EXTERNAL_INT_3,
	EXTERNAL_INT_4,
	EXTERNAL_INT_5,
	EXTERNAL_INT_6,
	EXTERNAL_INT_7,
	EXTERNAL_INT_8,
	EXTERNAL_INT_9,
	EXTERNAL_INT_10,
	EXTERNAL_INT_11,
	EXTERNAL_INT_12,
	EXTERNAL_INT_13,
	EXTERNAL_INT_14,
	EXTERNAL_INT_15,
	EXTERNAL_INT_NMI,
	EXTERNAL_NUM_INTERRUPTS,
	NOT_AN_INTERRUPT = -1,
	EXTERNAL_INT_NONE = NOT_AN_INTERRUPT,
} EExt_Interrupts ;

/* Types used for the table below */
typedef struct _PinDescription
{
	uint32_t       ulPort ;
	uint32_t        ulPin ;
	EExt_Interrupts		extin;
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

#elif defined(__SAMD11D14AM__) || defined(__SAMD09D14A__)

#define SEESAW_NUM_PINS 22

	const PinDescription g_APinDescription[]=
	{
		{ NO_PORT, 0, NOT_AN_INTERRUPT }, //NOT A PIN
		{ PORTA, 2, EXTERNAL_INT_2 },
		{ PORTA, 3, EXTERNAL_INT_3 },
		{ PORTA, 4, EXTERNAL_INT_4 }, //TC1 WO0
		{ PORTA, 5, EXTERNAL_INT_5 }, //TC1 WO1
		{ PORTA, 6, EXTERNAL_INT_6 }, //TC2 WO0
		{ PORTA, 7, EXTERNAL_INT_7 }, //TC2 WO1
		{ PORTA, 8, EXTERNAL_INT_6 },
		{ PORTA, 9, EXTERNAL_INT_7 },
		{ PORTA, 10, EXTERNAL_INT_2 },
		{ PORTA, 11, EXTERNAL_INT_3 },
		{ PORTA, 14, EXTERNAL_INT_NMI }, //Interrupt
		{ PORTA, 15, EXTERNAL_INT_1 },
		{ PORTA, 16, EXTERNAL_INT_0 }, //LED
		{ PORTA, 17, EXTERNAL_INT_1 },
		{ PORTA, 22, EXTERNAL_INT_6 }, //SDA
		{ PORTA, 23, EXTERNAL_INT_7 }, //SCL
		{ PORTA, 27, EXTERNAL_INT_7 },
		{ PORTA, 28, NOT_AN_INTERRUPT },
		{ PORTA, 30, EXTERNAL_INT_2 },
		{ PORTA, 31, EXTERNAL_INT_3 },
		{ PORTA, 24, EXTERNAL_INT_4 },
		{ PORTA, 25, EXTERNAL_INT_5 },
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