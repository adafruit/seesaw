#include "bsp_gpio.h"
#include "PinMap.h"

void pinPeripheral(uint8_t pin, uint32_t ulPeripheral){

    uint8_t port = PORTA;
#ifdef HAS_PORTB
    if(pin >= 32){
        port = PORTB;
        pin -= 32;
    }
#endif
	if ( pin & 1 ) // is pin odd?
	{
		uint32_t temp ;

		// Get whole current setup for both odd and even pins and remove odd one
		temp = (PORT->Group[port].PMUX[(uint32_t)pin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
		// Set new muxing
		PORT->Group[port].PMUX[(uint32_t)pin >> 1].reg = temp|PORT_PMUX_PMUXO( ulPeripheral ) ;
		// Enable port mux
		PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR ;
	}
	else // even pin
	{
		uint32_t temp ;

		temp = (PORT->Group[port].PMUX[(uint32_t)pin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
		PORT->Group[port].PMUX[(uint32_t)pin >> 1].reg = temp|PORT_PMUX_PMUXE( ulPeripheral ) ;
		PORT->Group[port].PINCFG[(uint32_t)pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR ; // Enable port mux
	}
}

void gpio_set_inen(uint32_t mask, uint8_t port)
{
	for(uint32_t i=0; i<32; i++){
		if( (mask & (1ul << i)) ){
			PORT->Group[port].PINCFG[i].bit.INEN = 1;
			PORT->Group[port].PINCFG[i].bit.DRVSTR = 1;
		}
	}
}

void gpio_pullenset_bulk(uint32_t mask, uint8_t port){
	for(uint32_t i=0; i<32; i++){
		if( (mask & ((uint32_t)1 << i)) ){
			PORT->Group[port].PINCFG[i].bit.PULLEN = 1;
		}
	}
}

void gpio_pullenclr_bulk(uint32_t mask, uint8_t port){
	for(uint32_t i=0; i<32; i++){
		if( (mask & ((uint32_t)1 << i)) ){
			PORT->Group[port].PINCFG[i].bit.PULLEN = 0;
		}
	}
}
