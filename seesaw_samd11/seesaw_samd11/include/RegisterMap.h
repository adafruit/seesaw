#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

//* ============== STATUS =================== *//
#define SEESAW_STATUS_BASE 0x00

#define SEESAW_STATUS_VERSION 0x02

#define SEESAW_STATUS_SWRST 0x7F

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

#define SEESAW_SERCOM0_BASE 0x02
#define SEESAW_SERCOM1_BASE 0x03
#define SEESAW_SERCOM2_BASE 0x04
#define SEESAW_SERCOM3_BASE 0x05
#define SEESAW_SERCOM4_BASE 0x06
#define SEESAW_SERCOM5_BASE 0x07

	/****** STATUS *****/
	#define SEESAW_SERCOM_STATUS 0x00

	//this is asserted when the sercom encorunters an error
	#define SEESAW_SERCOM_STATUS_ERROR_BITS 0x01

	//this flag gets set when there is data available in the RX fifo
	#define SEESAW_SERCOM_STATUS_DATA_RDY_BITS 0x02

	/****** INTEN *****/
	//writing a 1 to any of the bits in this register enables the interrupt
	#define SEESAW_SERCOM_INTEN 0x02

	// if this bit is set to 1, assert interrupt when data is ready in the rx fifo
	#define SEESAW_SERCOM_INTEN_DATA_RDY_BITS 0x01

	/****** INTENCLR *****/
	/* writing a 1 to any of these bits disables the interrupt.
	 * this automatically updates the inten register
	 */
	#define SEESAW_SERCOM_INTENCLR 0x03

	// writing a 1 disables the data rdy interrupt
	#define SEESAW_SERCOM_INTENCLR_DATA_RDY_BITS 0x01

	/****** BAUD *****/
	//this register sets the baud rate
	#define SEESAW_SERCOM_BAUD 0x04

	/****** INTCLR *****/
	//writing a 1 to any of these bits clears the interrupt
	#define SEESAW_SERCOM_INTCLR 0x05

	#define SEESAW_SERCOM_INTCLR_DATA_RDY 0x01

	/****** DATA *****/
	//for reading and writing data
	#define SEESAW_SERCOM_DATA 0x06
	
//* ================ Timer ===================== *//
#define SEESAW_TIMER_BASE 0x08

	/****** STATUS *****/
	#define SEESAW_TIMER_STATUS 0x00

	//this is asserted when this encorunters an error
	#define SEESAW_TIMER_STATUS_ERROR_BITS 0x01
	
	#define SEESAW_TIMER_PWM 0x01


//* ============== Interrupts =================== *//

#define SEESAW_INTERRUPT_SERCOM0_DATA_RDY ( (uint32_t)(1 << 0) )
#define SEESAW_INTERRUPT_SERCOM1_DATA_RDY ( (uint32_t)(1 << 1) )
#define SEESAW_INTERRUPT_SERCOM2_DATA_RDY ( (uint32_t)(1 << 2) )
#define SEESAW_INTERRUPT_SERCOM3_DATA_RDY ( (uint32_t)(1 << 3) )
#define SEESAW_INTERRUPT_SERCOM4_DATA_RDY ( (uint32_t)(1 << 4) )
#define SEESAW_INTERRUPT_SERCOM5_DATA_RDY ( (uint32_t)(1 << 5) )

#endif