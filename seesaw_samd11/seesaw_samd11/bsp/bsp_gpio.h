#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "sam.h"

inline void gpio_init(int port, int pin, int dir) { 
	PORT->Group[port].DIRSET.reg = ( (dir & 0x01) <<pin);
	PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);
}

inline void gpio_write(int port, int pin, int val) {
	if(val) PORT->Group[port].OUTSET.reg = (1<<pin);
	else PORT->Group[port].OUTCLR.reg = (1<<pin);
}

uint32_t gpio_get_hw_reg(uint32_t pmap);

void gpio_set_inen(uint32_t mask, uint8_t port = 0);

inline void gpio_dirset_bulk(int port, uint32_t mask) {
	PORT->Group[port].DIRSET.reg = mask;
	gpio_set_inen(mask);
}
inline void gpio_dirclr_bulk(int port, uint32_t mask) {
	PORT->Group[port].DIRCLR.reg = mask;
	gpio_set_inen(mask);
}

inline void gpio_outset_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTSET.reg = mask;
}

inline void gpio_outclr_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTCLR.reg = mask;
}

inline uint32_t gpio_read_bulk(int port){
	return PORT->Group[port].IN.reg;
}

uint32_t gpio_intenset(uint32_t pins);

uint32_t gpio_intenclr(uint32_t pins);

inline void gpio_toggle(int port, int pin) { PORT->Group[port].OUTTGL.reg = (1<<pin); }
	
void pinPeripheral(uint8_t pin, uint32_t ulPeripheral);

#endif