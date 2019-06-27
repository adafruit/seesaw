#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "sam.h"

void adc_init();
void adc_set_freerunning(bool mode);

inline void syncADC() { 
	#if defined(SAMD51)
	while(ADC0->SYNCBUSY.reg & ADC_SYNCBUSY_CTRLB); //wait for sync
	while(ADC1->SYNCBUSY.reg & ADC_SYNCBUSY_CTRLB); //wait for sync
	#else
	while (ADC->STATUS.bit.SYNCBUSY == 1); 
	#endif
	}
inline void adc_trigger(){ syncADC(); 
	#if defined(SAMD51)
	ADC0->SWTRIG.bit.START = 1;
	ADC1->SWTRIG.bit.START = 1;
	#else
	ADC->SWTRIG.bit.START = 1;
	#endif
	}

void adc_set_inputscan(uint8_t channels);

uint16_t adc_read(uint8_t channel);

void init_temp();

int32_t calculate_temperature();

#endif