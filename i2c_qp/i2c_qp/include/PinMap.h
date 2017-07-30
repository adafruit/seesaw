#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "sam.h"

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




#endif