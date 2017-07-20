#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#define SEESAW_STATUS_BASE 0x00

#define SEESAW_STATUS_VERSION 0x02

//* ============== GPIO =================== *//
#define SEESAW_GPIO_BASE 0x01

/* to set pinmode
* 0x01 <----------- high byte GPIO base
*      0x00 <----------- low byte pinmode command
*             (0x80 for output, 0x00 for input) | (7 bit pin number) <--  1 Data byte
*/

#define SEESAW_GPIO_PINMODE_CMD 0x00

#define SEESAW_GPIO_PINMODE_BITS 0x80
#define SEESAW_GPIO_PINMODE_INPUT_VAL 0x00
#define SEESAW_GPIO_PINMODE_OUTPUT_VAL 0x01

#define SEESAW_GPIO_PIN_BITS 0x7F

#define SEESAW_GPIO_GET_PINMODE_MODE(_cmd) ( ( _cmd & SEESAW_GPIO_PINMODE_BITS ) >> 7 )
#define SEESAW_GPIO_GET_PINMODE_PIN(_cmd) ( _cmd & SEESAW_GPIO_PIN_BITS )

#define SEESAW_GPIO_WRITE_CMD 0x01

#define SEESAW_GPIO_WRITE_VAL_BITS 0x80
#define SEESAW_GPIO_WRITE_PIN_BITS 0x7F

#define SEESAW_GPIO_GET_WRITE_VAL(_cmd) ( ( _cmd & SEESAW_GPIO_WRITE_VAL_BITS ) >> 7 )
#define SEESAW_GPIO_GET_WRITE_PIN(_cmd) ( _cmd & SEESAW_GPIO_WRITE_PIN_BITS )

#define SEESAW_GPIO_READ_CMD 0x02
#define SEESAW_GPIO_READ_PIN_BITS 0x7F
#define SEESAW_GPIO_GET_READ_PIN(_cmd) ( _cmd & SEESAW_GPIO_READ_PIN_BITS )

#define SEESAW_GPIO_TOGGLE_CMD 0x03
#define SEESAW_GPIO_TOGGLE_PIN_BITS 0x7F
#define SEESAW_GPIO_GET_TOGGLE_PIN(_cmd) ( _cmd & SEESAW_GPIO_TOGGLE_PIN_BITS )

//* ============== SERCOM =================== *//
#define SEESAW_SERCOM_BASE 0x02

#define SEESAW_SERCOM_DATA 0x01
#define SEESAW_SERCOM_BAUD 0x02

#endif