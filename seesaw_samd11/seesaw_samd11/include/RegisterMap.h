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

	#define SEESAW_GPIO_PINMODE_SINGLE 0x00
		#define SEESAW_GPIO_PINMODE_INPUT_VAL 0x00
		#define SEESAW_GPIO_PINMODE_OUTPUT_VAL 0x01
		
	/****** SINGLE PIN *****/
	/* read or write the value of a single pin */
	#define SEESAW_GPIO_PIN_SINGLE 0x01
	
	/****** BULK PINMODE *****/
	/* set pinmode of multiple pins
	*/
	#define SEESAW_GPIO_PINMODE_BULK 0x02
	
	/****** BULK *****/
	/* Read or write multiple pins at once. 1 is set, 0 is cleared. 
	*/
	#define SEESAW_GPIO_BULK 0x03
	
	/****** BULK SET *****/
	/* Set multiple pins at once. Writing a 1 to any bit sets the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_SET 0x04
	
	/****** BULK CLEAR *****/
	/* Clear multiple pins at once. Writing a 1 to any bit clears the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_CLR 0x05
	
	/****** BULK TOGGLE *****/
	/* Toggle multiple pins at once. Writing a 1 to any bit toggles the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_TOGGLE 0x06

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
	
	/****** PWM *****/
	#define SEESAW_TIMER_PWM 0x01
	
//* ============== ADC =================== *//
#define SEESAW_ADC_BASE 0x09

	/****** STATUS *****/
	#define SEESAW_ADC_STATUS 0x00

	//this is asserted when this encorunters an error
	#define SEESAW_ADC_STATUS_ERROR_BITS 0x01
	
	//this is asserted when window monitor interrupt occurs
	#define SEESAW_ADC_STATUS_WINMON 0x02
	
	/****** INTENSET *****/
	//writing a 1 to any of the bits in this register enables the interrupt
	#define SEESAW_ADC_INTEN 0x02

	// if this bit is set to 1, assert interrupt when window monitor conditions are met
	#define SEESAW_ADC_INTEN_WINMON_BITS 0x01

	/****** INTENCLR *****/
	/* writing a 1 to any of these bits disables the interrupt.
	 * this automatically updates the inten register
	 */
	#define SEESAW_ADC_INTENCLR 0x03

	// writing a 1 disables the window monitor interrupt
	#define SEESAW_ADC_INTENCLR_WINMON_BITS 0x01
	
	/****** WINMODE *****/
	#define SEESAW_ADC_WINMODE 0x04
		
	/****** WINTHRESH *****/
	//window monitor thresholds
	#define SEESAW_ADC_WINTHRESH 0x05
	
	/****** INTCLR *****/
	//writing a 1 to any of these bits clears the interrupt
	#define SEESAW_ADC_INTCLR 0x06

	#define SEESAW_ADC_INTCLR_WINMON 0x01
	
	/****** CHANNELS *****/
	#define SEESAW_ADC_CHANNEL_0 0x07
	#define SEESAW_ADC_CHANNEL_1 0x08
	#define SEESAW_ADC_CHANNEL_2 0x09
	#define SEESAW_ADC_CHANNEL_3 0x0A
	#define SEESAW_ADC_CHANNEL_4 0x0B


//* ============== Interrupts =================== *//

#define SEESAW_INTERRUPT_SERCOM0_DATA_RDY ( (uint32_t)(1 << 0) )
#define SEESAW_INTERRUPT_SERCOM1_DATA_RDY ( (uint32_t)(1 << 1) )
#define SEESAW_INTERRUPT_SERCOM2_DATA_RDY ( (uint32_t)(1 << 2) )
#define SEESAW_INTERRUPT_SERCOM3_DATA_RDY ( (uint32_t)(1 << 3) )
#define SEESAW_INTERRUPT_SERCOM4_DATA_RDY ( (uint32_t)(1 << 4) )
#define SEESAW_INTERRUPT_SERCOM5_DATA_RDY ( (uint32_t)(1 << 5) )

#endif