#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "sam.h"

inline void gpio_init(int port, int pin, int dir) { PORT->Group[port].DIRSET.reg = ( (dir & 0x01) <<pin); }

void gpio_write(int port, int pin, int val) {
	if(val) PORT->Group[port].OUTSET.reg = (1<<pin);
	else PORT->Group[port].OUTCLR.reg = (1<<pin);
}

inline void gpio_toggle(int port, int pin) { PORT->Group[port].OUTTGL.reg = (1<<pin); }

#endif