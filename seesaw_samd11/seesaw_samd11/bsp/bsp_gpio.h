#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "sam.h"

inline void gpio_init(int port, int pin, int dir) { PORT->Group[port].DIRSET.reg = ( (dir & 0x01) <<pin); }

inline void gpio_write(int port, int pin, int val) {
	if(val) PORT->Group[port].OUTSET.reg = (1<<pin);
	else PORT->Group[port].OUTCLR.reg = (1<<pin);
}

uint32_t gpio_get_hw_reg(uint32_t pmap);

inline void gpio_pinmode_bulk(int port, uint32_t mask) {
	PORT->Group[port].DIRSET.reg = mask;
}

inline void gpio_outset_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTSET.reg = mask;
}

inline void gpio_outclr_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTCLR.reg = mask;
}

inline void gpio_toggle(int port, int pin) { PORT->Group[port].OUTTGL.reg = (1<<pin); }
	
void pinPeripheral(uint8_t pin, uint32_t ulPeripheral);

#endif