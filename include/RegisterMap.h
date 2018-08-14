#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

//* ============== STATUS =================== *//
#define SEESAW_STATUS_BASE 0x00

	#define SEESAW_STATUS_HW_ID 0x01
		#define SEESAW_STATUS_HW_ID_CODE 0x55

	#define SEESAW_STATUS_VERSION 0x02
	
	#define SEESAW_STATUS_OPTIONS 0x03

	#define SEESAW_STATUS_SWRST 0x7F

//* ============== GPIO =================== *//
#define SEESAW_GPIO_BASE 0x01

/* to set pinmode
* 0x01 <----------- high byte GPIO base
*      0x00 <----------- low byte pinmode command
*/

	#define SEESAW_GPIO_PINMODE_SINGLE 0x00
		#define SEESAW_GPIO_PINMODE_INPUT_VAL 0x00
		#define SEESAW_GPIO_PINMODE_OUTPUT_VAL 0x01
		
	/****** SINGLE PIN *****/
	/* read or write the value of a single pin */
	#define SEESAW_GPIO_PIN_SINGLE 0x01
	
	/****** BULK DIRSET *****/
	/* set set multiple pins to output
	*/
	#define SEESAW_GPIO_DIRSET_BULK 0x02
	
	/****** BULK DIRCLR *****/
	/* set set multiple pins to input
	*/
	#define SEESAW_GPIO_DIRCLR_BULK 0x03
	
	/****** BULK *****/
	/* Read or write multiple pins at once. 1 is set, 0 is cleared. 
	*/
	#define SEESAW_GPIO_BULK 0x04
	
	/****** BULK SET *****/
	/* Set multiple pins at once. Writing a 1 to any bit sets the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_SET 0x05
	
	/****** BULK CLEAR *****/
	/* Clear multiple pins at once. Writing a 1 to any bit clears the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_CLR 0x06
	
	/****** BULK TOGGLE *****/
	/* Toggle multiple pins at once. Writing a 1 to any bit toggles the corresponding pin 
	*/
	#define SEESAW_GPIO_BULK_TOGGLE 0x07
	
	/****** INTENSET *****/
	/* Writing a 1 to any of these bits enables the interrupt on that pin */
	#define SEESAW_GPIO_INTENSET	0x08
	
	/****** INTENCLR *****/
	/* Writing a 1 to any of these bits disables the interrupt on that pin */
	#define SEESAW_GPIO_INTENCLR	0x09
	
	/****** INTFLAG *****/
	/* these flags get set when an interrupt fires on a pin. Reading this register clears all interrupts. */
	#define SEESAW_GPIO_INTFLAG	0x0A
	
	/****** PULLENSET *****/
	/* Writing a 1 to any of these bits enables pull on that pin */
	#define SEESAW_GPIO_PULLENSET 0x0B
	
	/****** PULLENCLR *****/
	/* Writing a 1 to any of these bits disables pull on that pin */
	#define SEESAW_GPIO_PULLENCLR 0x0C
	
	
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

	/****** DATA *****/
	//for reading and writing data
	#define SEESAW_SERCOM_DATA 0x05
	
//* ================ Timer ===================== *//
#define SEESAW_TIMER_BASE 0x08

	/****** STATUS *****/
	#define SEESAW_TIMER_STATUS 0x00

	//this is asserted when this encorunters an error
	#define SEESAW_TIMER_STATUS_ERROR_BITS 0x01
	
	/****** PWM *****/
	#define SEESAW_TIMER_PWM 0x01
	
	/****** FREQ ****/
	#define SEESAW_TIMER_FREQ 0x02
	
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
	
	/****** CHANNELS *****/
	#define SEESAW_ADC_CHANNEL_0 0x07
	#define SEESAW_ADC_CHANNEL_1 0x08
	#define SEESAW_ADC_CHANNEL_2 0x09
	#define SEESAW_ADC_CHANNEL_3 0x0A
    #define SEESAW_ADC_CHANNEL_4 0x0B
    #define SEESAW_ADC_CHANNEL_5 0x0C
    #define SEESAW_ADC_CHANNEL_6 0x0D
    #define SEESAW_ADC_CHANNEL_7 0x0E
	
//* ============== DAC =================== *//
#define SEESAW_DAC_BASE 0x0A

	/****** STATUS *****/
	#define SEESAW_DAC_STATUS 0x00

//* ============== Interrupts =================== *//
#define SEESAW_INTERRUPT_BASE 0x0B

//TODO: open drain/push-pull, or inversion control register

#define SEESAW_INTERRUPT_GPIO			  ( (uint32_t)(1ul << 0) )
#define SEESAW_INTERRUPT_SERCOM0_DATA_RDY ( (uint32_t)(1ul << 1) )
#define SEESAW_INTERRUPT_SERCOM1_DATA_RDY ( (uint32_t)(1ul << 2) )
#define SEESAW_INTERRUPT_SERCOM2_DATA_RDY ( (uint32_t)(1ul << 3) )
#define SEESAW_INTERRUPT_SERCOM3_DATA_RDY ( (uint32_t)(1ul << 4) )
#define SEESAW_INTERRUPT_SERCOM4_DATA_RDY ( (uint32_t)(1ul << 5) )
#define SEESAW_INTERRUPT_SERCOM5_DATA_RDY ( (uint32_t)(1ul << 6) )
#define SEESAW_INTERRUPT_KEYPAD_DATA_RDY  ( (uint32_t)(1ul << 7) )
#define SEESAW_INTERRUPT_SPI_SLAVE_DATA_RDY ( (uint32_t)(1ul << 8) )

//* ============== DAP =================== *//
#define SEESAW_DAP_BASE 0x0C

//* ============== EEPROM =================== *//
#define SEESAW_EEPROM_BASE 0x0D

	#define SEESAW_EEPROM_I2C_ADDR 0x3F

//* ============== NEOPIXEL =================== *//
#define SEESAW_NEOPIXEL_BASE 0x0E

	/****** STATUS *****/
	#define SEESAW_NEOPIXEL_STATUS 0x00
	
	/****** PIN *****/
	#define SEESAW_NEOPIXEL_PIN 0x01
	
	/****** SPEED *****/
	#define SEESAW_NEOPIXEL_SPEED 0x02
	
	/****** BUFFER LENGTH *****/
	#define SEESAW_NEOPIXEL_BUF_LENGTH 0x03
	
	/****** BUFFER DATA *****/
	#define SEESAW_NEOPIXEL_BUF 0x04
	
	/****** SHOW *****/
	#define SEESAW_NEOPIXEL_SHOW 0x05

#endif

//* ============== TOUCH =================== *//
#define SEESAW_TOUCH_BASE 0x0F

    /****** CHANNELS *****/
    #define SEESAW_TOUCH_CHANNEL_0 0x10
    #define SEESAW_TOUCH_CHANNEL_1 0x11
    #define SEESAW_TOUCH_CHANNEL_2 0x12
    #define SEESAW_TOUCH_CHANNEL_3 0x13

//* ============== KEYPAD =================== *//
#define SEESAW_KEYPAD_BASE 0x10

	/****** STATUS *****/
	#define SEESAW_KEYPAD_STATUS 0x00

	/****** Enable or disable row/col events *****/
	#define SEESAW_KEYPAD_EVENT 0x01

	/****** Writing a 1 to any bit in this register will enable the interrupt *****/
	#define SEESAW_KEYPAD_INTENSET 0x02

	/****** Writing a 1 to any bit in this register will disable the interrupt *****/
	#define SEESAW_KEYPAD_INTENCLR 0x03

	/****** number of events in the fifo *****/
	#define SEESAW_KEYPAD_COUNT 0x04

	/****** Reading data from the fifo will clear interrupt and data ready flag *****/
	#define SEESAW_KEYPAD_FIFO 0x10
