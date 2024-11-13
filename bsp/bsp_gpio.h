#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "sam.h"

inline void gpio_init(int port, int pin, int dir) {
	if(dir)
		PORT->Group[port].DIRSET.reg = ( 1UL<<pin);
	else 
		PORT->Group[port].DIRCLR.reg = ( 1UL<<pin);
		
	PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);
}

inline void gpio_reset_eic(){
	EIC->CTRL.bit.SWRST = 1;
	while(EIC->CTRL.bit.SWRST || EIC->STATUS.bit.SYNCBUSY);
}

inline void gpio_write(int port, int pin, int val) {
	if(val) PORT->Group[port].OUTSET.reg = (1ul<<pin);
	else PORT->Group[port].OUTCLR.reg = (1ul<<pin);
}

uint32_t gpio_get_hw_reg(uint32_t pmap);

void gpio_set_inen(uint32_t mask, uint8_t port = 0);

inline void gpio_dirset_bulk(int port, uint32_t mask) {
	PORT->Group[port].DIRSET.reg = mask;
	gpio_set_inen(mask, port);
}
inline void gpio_dirclr_bulk(int port, uint32_t mask) {
	PORT->Group[port].DIRCLR.reg = mask;
	gpio_set_inen(mask, port);
}

inline void gpio_outset_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTSET.reg = mask;
}

inline void gpio_outclr_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTCLR.reg = mask;
}

inline void gpio_toggle_bulk(int port, uint32_t mask) {
	PORT->Group[port].OUTTGL.reg = mask;
}

void gpio_pullenset_bulk(uint32_t mask, uint8_t port = 0);

void gpio_pullenclr_bulk(uint32_t mask, uint8_t port = 0);

inline uint32_t gpio_read_bulk(int port = 0){
	return PORT->Group[port].IN.reg;
}

uint32_t gpio_intenset(uint32_t pins);

uint32_t gpio_intenclr(uint32_t pins);

inline void gpio_toggle(int port, int pin) { PORT->Group[port].OUTTGL.reg = (1ul<<pin); }
	
void pinPeripheral(uint8_t pin, uint32_t ulPeripheral);

#endif
