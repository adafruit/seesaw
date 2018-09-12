#include "bsp_neopix.h"

void neopix_show_400k(uint32_t pin, uint8_t *pixels, uint16_t numBytes)
{

    uint8_t  *ptr, *end, p, bitMask;
    uint32_t  pinMask;

    volatile uint32_t *set = &(PORT->Group[PORTA].OUTSET.reg);
    volatile uint32_t *clr = &(PORT->Group[PORTA].OUTCLR.reg);
#ifdef HAS_PORTB
    if(pin >= 32) {
        set = &(PORT->Group[PORTB].OUTSET.reg);
        clr = &(PORT->Group[PORTB].OUTCLR.reg);
        pin -= 32;
    }
#endif

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

void neopix_show_800k(uint32_t pin, uint8_t *pixels, uint16_t numBytes)
{
    uint8_t  *ptr, *end, p, bitMask;
    uint32_t  pinMask;

    pinMask =  1ul << pin;
    ptr     =  pixels;
    end     =  ptr + numBytes;
    p       = *ptr++;
    bitMask =  0x80;

    volatile uint32_t *set = &(PORT->Group[PORTA].OUTSET.reg);
    volatile uint32_t *clr = &(PORT->Group[PORTA].OUTCLR.reg);
#ifdef HAS_PORTB
    if(pin >= 32) {
        set = &(PORT->Group[PORTB].OUTSET.reg);
        clr = &(PORT->Group[PORTB].OUTCLR.reg);
        pin -= 32;
    }
#endif

    for(;;) {
#if defined(SAMD21)
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
#else
        *set = pinMask;
        asm("nop; nop; nop; nop; nop; nop;");
        if(p & bitMask) {
            asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;");
            *clr = pinMask;
        } else {
            *clr = pinMask;
            asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;");
        }
        if(bitMask >>= 1) {
            asm("nop; nop; nop; nop; nop; nop; nop;");
        } else {
            if(ptr >= end) break;
            p       = *ptr++;
            bitMask = 0x80;
        }
#endif
    }
}
