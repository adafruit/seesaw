#include "bsp_gpio.h"
#include "PinMap.h"

volatile uint32_t *set = &(PORT->Group[PORTA].OUTSET.reg);
volatile uint32_t *clr = &(PORT->Group[PORTA].OUTCLR.reg);

void pinPeripheral(uint8_t pin, uint32_t ulPeripheral){
	
	if ( pin & 1 ) // is pin odd?
	{
		uint32_t temp ;

		// Get whole current setup for both odd and even pins and remove odd one
		temp = (PORT->Group[PORTA].PMUX[(uint32_t)pin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
		// Set new muxing
		PORT->Group[PORTA].PMUX[(uint32_t)pin >> 1].reg = temp|PORT_PMUX_PMUXO( ulPeripheral ) ;
		// Enable port mux
		PORT->Group[PORTA].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN ;
	}
	else // even pin
	{
		uint32_t temp ;

		temp = (PORT->Group[PORTA].PMUX[(uint32_t)pin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
		PORT->Group[PORTA].PMUX[(uint32_t)pin >> 1].reg = temp|PORT_PMUX_PMUXE( ulPeripheral ) ;
		PORT->Group[PORTA].PINCFG[(uint32_t)pin].reg |= PORT_PINCFG_PMUXEN ; // Enable port mux
	}
}

void gpio_set_inen(uint32_t mask, uint8_t port)
{
	for(uint32_t i=0; i<32; i++){
		if( (mask & (1ul << i)) ){
			PORT->Group[port].PINCFG[i].bit.INEN = 1;
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

//TODO: delete, this shouldn't be needed
/*
uint32_t gpio_get_hw_reg(uint32_t pmap)
{
	uint32_t hw_reg = 0;
	for(uint32_t i=0; i<32; i++){
		if( (pmap & (1ul << i)) && g_APinDescription[i].ulPort == PORTA){
			hw_reg |= (1 << g_APinDescription[i].ulPin);
		}
	}
	return hw_reg;
}

uint32_t gpio_intenset(uint32_t pins)
{
	uint32_t ret = 0;
	for(uint32_t i=0; i<SEESAW_NUM_PINS; i++){
		if( (pins & (1ul << i)) ){
			EExt_Interrupts in = g_APinDescription[i].extin;
			
			ret |= (1ul << in);
			EIC->WAKEUP.reg |= (1 << in);
			pinPeripheral(i, 0);
			
			//rising and falling interrupt
			EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE0_BOTH_Val << (in << 2);
			EIC->INTENSET.reg = EIC_INTENSET_EXTINT(1 << in);
		}
	}
	return ret;
}

uint32_t gpio_intenclr(uint32_t pins)
{
	uint32_t ret = 0;
	for(uint32_t i=0; i<SEESAW_NUM_PINS; i++){
		if( (pins & (1ul << i)) ){
			EExt_Interrupts in = g_APinDescription[i].extin;
			ret |= (1ul << in);
			EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT(1 << in);
			EIC->WAKEUP.reg &= ~(1 << in);
		}
	}
	return ret;
}
*/