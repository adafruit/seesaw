#include "bsp_sercom.h"

bool sendDataSlaveWIRE( Sercom *sercom, uint8_t data )
{
	//Send data
	sercom->I2CS.DATA.bit.DATA = data;

	//Problems on line? nack received?
	if(!sercom->I2CS.INTFLAG.bit.DRDY || sercom->I2CS.STATUS.bit.RXNACK)
	return false;
	else
	return true;
}

int availableWIRE(  Sercom *sercom )
{
	if(isMasterWIRE(sercom))
	return sercom->I2CM.INTFLAG.bit.SB;
	else
	return sercom->I2CS.INTFLAG.bit.DRDY;
}

void prepareNackBitWIRE( Sercom *sercom )
{
	if(isMasterWIRE(sercom)) {
		// Send a NACK
		sercom->I2CM.CTRLB.bit.ACKACT = 1;
		} else {
		sercom->I2CS.CTRLB.bit.ACKACT = 1;
	}
}

void prepareAckBitWIRE( Sercom *sercom )
{
	if(isMasterWIRE(sercom)) {
		// Send an ACK
		sercom->I2CM.CTRLB.bit.ACKACT = 0;
		} else {
		sercom->I2CS.CTRLB.bit.ACKACT = 0;
	}
}

void prepareCommandBitsWire(Sercom *sercom, uint8_t cmd)
{
	if(isMasterWIRE(sercom)) {
		sercom->I2CM.CTRLB.bit.CMD = cmd;

		while(sercom->I2CM.SYNCBUSY.bit.SYSOP)
		{
			// Waiting for synchronization
		}
		} else {
		sercom->I2CS.CTRLB.bit.CMD = cmd;
	}
}

uint8_t readDataWIRE(  Sercom *sercom )
{
	if(isMasterWIRE(sercom))
	{
		while( sercom->I2CM.INTFLAG.bit.SB == 0 )
		{
			// Waiting complete receive
		}

		return sercom->I2CM.DATA.bit.DATA ;
	}
	else
	{
		return sercom->I2CS.DATA.reg ;
	}
}

void initClock( Sercom *sercom )
{
	uint8_t clockId = 0;

	if(sercom == SERCOM0)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM0_CORE;
	}
	else if(sercom == SERCOM1)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM1_CORE;
	}

#if defined(SERCOM2)
	else if(sercom == SERCOM2)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM2_CORE;
	}
#endif

#if defined(SERCOM3)
	else if(sercom == SERCOM3)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM3_CORE;
	}
#endif

#if defined(SERCOM4)
	else if(sercom == SERCOM4)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM4_CORE;
	}
#endif

#if defined(SERCOM5)
	else if(sercom == SERCOM5)
	{
		clockId = GCLK_CLKCTRL_ID_SERCOM5_CORE;
}
#endif

	//Setting clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( clockId ) | // Generic Clock 0 (SERCOMx)
	GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
	GCLK_CLKCTRL_CLKEN ;
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOMX_SLOW ) |
	GCLK_CLKCTRL_GEN_GCLK1 |
	GCLK_CLKCTRL_CLKEN ;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
		/* Wait for synchronization */
	}
}


/*	=========================
 *	===== Sercom WIRE
 *	=========================
 */
void resetWIRE(Sercom *sercom)
{	
  //I2CM OR I2CS, no matter SWRST is the same bit.

  //Setting the Software bit to 1
  sercom->I2CM.CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while(sercom->I2CM.CTRLA.bit.SWRST || sercom->I2CM.SYNCBUSY.bit.SWRST);
}

void enableWIRE(Sercom *sercom)
{
  
  // Enable the I²C master mode
  sercom->I2CM.CTRLA.bit.ENABLE = 1 ;

  while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 )
  {
    // Waiting the enable bit from SYNCBUSY is equal to 0;
  }

  // Setting bus idle mode
  sercom->I2CM.STATUS.bit.BUSSTATE = 1 ;

  while ( sercom->I2CM.SYNCBUSY.bit.SYSOP != 0 )
  {
    // Wait the SYSOP bit from SYNCBUSY coming back to 0
  }
}

void initSlaveWIRE( Sercom *sercom, uint8_t ucAddress )
{
  // Initialize the peripheral clock and interruption
  initClock(sercom) ;
  resetWIRE(sercom) ;

  // Set slave mode
  sercom->I2CS.CTRLA.bit.MODE = I2C_SLAVE_OPERATION;

  sercom->I2CS.ADDR.reg = SERCOM_I2CS_ADDR_ADDR( ucAddress & 0x7Ful ) | // 0x7F, select only 7 bits
                          SERCOM_I2CS_ADDR_ADDRMASK( 0x00ul ) ;         // 0x00, only match exact address

  enableInterruptsWIRE( sercom );

  while ( sercom->I2CM.SYNCBUSY.bit.SYSOP != 0 )
  {
    // Wait the SYSOP bit from SYNCBUSY to come back to 0
  }
}

/* 	=========================
 *	===== Sercom UART
 *	=========================
*/

void resetUART( Sercom * sercom )
{
	// Start the Software Reset
	sercom->USART.CTRLA.bit.SWRST = 1 ;

	while ( sercom->USART.CTRLA.bit.SWRST || sercom->USART.SYNCBUSY.bit.SWRST )
	{
		// Wait for both bits Software Reset from CTRLA and SYNCBUSY coming back to 0
	}
} 

bool isDataRegisterEmptyUART( Sercom * sercom )
{
	//DRE : Data Register Empty
	return sercom->USART.INTFLAG.bit.DRE;
}

void initUART( Sercom * sercom, SercomUartSampleRate sampleRate, uint32_t baudrate)
{
  initClock(sercom);
  resetUART(sercom);

  //Setting the CTRLA register
  sercom->USART.CTRLA.reg =	SERCOM_USART_CTRLA_MODE(0x01) |
                SERCOM_USART_CTRLA_SAMPR(sampleRate);

  //Setting the Interrupt register
  sercom->USART.INTENSET.reg =	SERCOM_USART_INTENSET_RXC |  //Received complete
                                SERCOM_USART_INTENSET_ERROR; //All others errors

	uint16_t sampleRateValue;

	if (sampleRate == SAMPLE_RATE_x16) {
		sampleRateValue = 16;
	} else {
		sampleRateValue = 8;
	}

	// Asynchronous fractional mode (Table 24-2 in datasheet)
	//   BAUD = fref / (sampleRateValue * fbaud)
	// (multiply by 8, to calculate fractional piece)
	uint32_t baudTimes8 = (SystemCoreClock * 8) / (sampleRateValue * baudrate);

	sercom->USART.BAUD.FRAC.FP   = (baudTimes8 % 8);
	sercom->USART.BAUD.FRAC.BAUD = (baudTimes8 / 8);
}
void initFrame( Sercom * sercom , SercomUartCharSize charSize, SercomDataOrder dataOrder, SercomParityMode parityMode, SercomNumberStopBit nbStopBits)
{
  //Setting the CTRLA register
  sercom->USART.CTRLA.reg |=	SERCOM_USART_CTRLA_FORM( (parityMode == SERCOM_NO_PARITY ? 0 : 1) ) |
                dataOrder << SERCOM_USART_CTRLA_DORD_Pos;

  //Setting the CTRLB register
  sercom->USART.CTRLB.reg |=	SERCOM_USART_CTRLB_CHSIZE(charSize) |
                nbStopBits << SERCOM_USART_CTRLB_SBMODE_Pos |
                (parityMode == SERCOM_NO_PARITY ? 0 : parityMode) << SERCOM_USART_CTRLB_PMODE_Pos; //If no parity use default value
}

void initPads( Sercom * sercom , SercomUartTXPad txPad, SercomRXPad rxPad)
{
  //Setting the CTRLA register
  sercom->USART.CTRLA.reg |=	SERCOM_USART_CTRLA_TXPO(txPad) |
                SERCOM_USART_CTRLA_RXPO(rxPad);

  // Enable Transceiver and Receiver
  sercom->USART.CTRLB.reg |= SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN ;
}

int writeDataUART( Sercom * sercom ,uint8_t data)
{
	// Wait for data register to be empty
	while(!isDataRegisterEmptyUART(sercom));

	//Put data into DATA register
	sercom->USART.DATA.reg = (uint16_t)data;
	return 1;
}

void enableUART( Sercom * sercom )
{
	while(sercom->USART.SYNCBUSY.bit.ENABLE || sercom->USART.SYNCBUSY.bit.SWRST);
	
	//Setting  the enable bit to 1
	sercom->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;

	//Wait for then enable bit from SYNCBUSY is equal to 0;
	while(sercom->USART.SYNCBUSY.bit.ENABLE);
}

void disableUART( Sercom *sercom )
{
	while(sercom->USART.SYNCBUSY.bit.ENABLE || sercom->USART.SYNCBUSY.bit.SWRST);
	
	//Setting  the enable bit to 0
	sercom->USART.CTRLA.bit.ENABLE = 0;

	//Wait for then enable bit from SYNCBUSY is equal to 0;
	while(sercom->USART.SYNCBUSY.bit.ENABLE);
}

void setUARTBaud( Sercom *sercom, uint32_t baudrate )
{
	disableUART(sercom);
	
	// Asynchronous fractional mode (Table 24-2 in datasheet)
	//   BAUD = fref / (sampleRateValue * fbaud)
	// (multiply by 8, to calculate fractional piece)
	uint32_t baudTimes8 = (SystemCoreClock * 8) / (16 * baudrate);

	sercom->USART.BAUD.FRAC.FP   = (baudTimes8 % 8);
	sercom->USART.BAUD.FRAC.BAUD = (baudTimes8 / 8);
	
	enableUART(sercom);
}
