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
#define PIN_USB_DM 24
#define PIN_USB_DP 25

extern volatile uint32_t *set;
extern volatile uint32_t *clr;

#endif