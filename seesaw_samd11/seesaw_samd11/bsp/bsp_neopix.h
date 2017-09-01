#ifndef LIB_BSP_NEOPIX_H
#define LIB_BSP_NEOPIX_H

#include "PinMap.h"

static void neopix_show_400k(uint32_t pin, uint8_t *pixels, uint16_t numBytes)
{

	uint8_t  *ptr, *end, p, bitMask, portNum;
	uint32_t  pinMask;

	pinMask =  1ul << pin;
	ptr     =  pixels;
	end     =  ptr + numBytes;
	p       = *ptr++;
	bitMask =  0x80;

	for(;;) {
		*set = pinMask;
		asm("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
		if(p & bitMask) {
			asm("nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop;");
			*clr = pinMask;
			} else {
			*clr = pinMask;
			asm("nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop;");
		}
		asm("nop; nop; nop; nop; nop; nop; nop; nop;"
		"nop; nop; nop; nop; nop; nop; nop; nop;"
		"nop; nop; nop; nop; nop; nop; nop; nop;"
		"nop; nop; nop; nop; nop; nop; nop; nop;");
		if(bitMask >>= 1) {
			asm("nop; nop; nop; nop; nop; nop; nop;");
			} else {
			if(ptr >= end) break;
			p       = *ptr++;
			bitMask = 0x80;
		}
	}
}

static void neopix_show_800k(uint32_t pin, uint8_t *pixels, uint16_t numBytes)
{
	uint8_t  *ptr, *end, p, bitMask, portNum;
	uint32_t  pinMask;

	pinMask =  1ul << pin;
	ptr     =  pixels;
	end     =  ptr + numBytes;
	p       = *ptr++;
	bitMask =  0x80;
	
	for(;;) {
		*set = pinMask;
		asm("nop; nop; nop; nop; nop; nop; nop; nop;");
		if(p & bitMask) {
			asm("nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop;");
			*clr = pinMask;
			} else {
			*clr = pinMask;
			asm("nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop; nop; nop; nop; nop;"
			"nop; nop; nop; nop;");
		}
		if(bitMask >>= 1) {
			asm("nop; nop; nop; nop; nop; nop; nop; nop; nop;");
			} else {
			if(ptr >= end) break;
			p       = *ptr++;
			bitMask = 0x80;
		}
	}
}

#endif
