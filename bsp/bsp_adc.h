#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "sam.h"

void adc_init();
void adc_set_freerunning(bool mode);

inline void syncADC() { while (ADC->STATUS.bit.SYNCBUSY == 1); }
inline void adc_trigger(){ syncADC(); ADC->SWTRIG.bit.START = 1;}

void adc_set_inputscan(uint8_t channels);

uint16_t adc_read(uint8_t channel);

void init_temp();

int32_t calculate_temperature();

#endif