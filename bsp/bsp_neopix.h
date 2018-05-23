#ifndef LIB_BSP_NEOPIX_H
#define LIB_BSP_NEOPIX_H

#include "PinMap.h"

extern void neopix_show_400k(uint32_t pin, uint8_t *pixels, uint16_t numBytes);
extern void neopix_show_800k(uint32_t pin, uint8_t *pixels, uint16_t numBytes);

#endif
