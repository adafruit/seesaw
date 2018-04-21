#ifndef BSP_SERCOM_H
#define BSP_SERCOM_H

#include "sam.h"

#define I2C_SLAVE_OPERATION 0x4u
#define I2C_MASTER_OPERATION 0x5u

#define SERCOM_FREQ_REF 48000000ul

typedef enum
{
	SPI_SLAVE_OPERATION = 0x2u,
	SPI_MASTER_OPERATION = 0x3u
} SercomSpiMode;

enum
{
	WIRE_UNKNOWN_STATE = 0x0ul,
	WIRE_IDLE_STATE,
	WIRE_OWNER_STATE,
	WIRE_BUSY_STATE
};

typedef enum
{
	SERCOM_EVEN_PARITY = 0,
	SERCOM_ODD_PARITY,
	SERCOM_NO_PARITY
} SercomParityMode;

typedef enum
{
	SERCOM_STOP_BIT_1 = 0,
	SERCOM_STOP_BITS_2
} SercomNumberStopBit;

typedef enum
{
	MSB_FIRST = 0,
	LSB_FIRST
} SercomDataOrder;

typedef enum
{
	UART_CHAR_SIZE_8_BITS = 0,
	UART_CHAR_SIZE_9_BITS,
	UART_CHAR_SIZE_5_BITS = 0x5u,
	UART_CHAR_SIZE_6_BITS,
	UART_CHAR_SIZE_7_BITS
} SercomUartCharSize;

typedef enum
{
	SERCOM_RX_PAD_0 = 0,
	SERCOM_RX_PAD_1,
	SERCOM_RX_PAD_2,
	SERCOM_RX_PAD_3,
} SercomRXPad;

typedef enum
{
	UART_TX_PAD_0 = 0x0ul,	// Only for UART
	UART_TX_PAD_2 = 0x1ul,  // Only for UART
	UART_TX_RTS_CTS_PAD_0_2_3 = 0x2ul,  // Only for UART with TX on PAD0, RTS on PAD2 and CTS on PAD3
} SercomUartTXPad;

typedef enum
{
	SAMPLE_RATE_x16 = 0x1,	//Fractional
	SAMPLE_RATE_x8 = 0x3,	//Fractional
} SercomUartSampleRate;

typedef enum
{
	SERCOM_SPI_MODE_0 = 0,	// CPOL : 0  | CPHA : 0
	SERCOM_SPI_MODE_1,		// CPOL : 0  | CPHA : 1
	SERCOM_SPI_MODE_2,		// CPOL : 1  | CPHA : 0
	SERCOM_SPI_MODE_3		// CPOL : 1  | CPHA : 1
} SercomSpiClockMode;

typedef enum
{
	SPI_PAD_0_SCK_1 = 0,
	SPI_PAD_2_SCK_3,
	SPI_PAD_3_SCK_1,
	SPI_PAD_0_SCK_3
} SercomSpiTXPad;

typedef enum
{
	SPI_CHAR_SIZE_8_BITS = 0x0ul,
	SPI_CHAR_SIZE_9_BITS
} SercomSpiCharSize;

bool sendDataSlaveWIRE( Sercom *sercom, uint8_t data );

inline bool isMasterWIRE(  Sercom *sercom )
{
	return sercom->I2CS.CTRLA.bit.MODE == I2C_MASTER_OPERATION;
}

inline bool isSlaveWIRE(  Sercom *sercom )
{
	return sercom->I2CS.CTRLA.bit.MODE == I2C_SLAVE_OPERATION;
}

inline bool isBusIdleWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_IDLE_STATE;
}

inline bool isBusOwnerWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_OWNER_STATE;
}

inline bool isDataReadyWIRE(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.DRDY;
}

inline bool isStopDetectedWIRE(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.PREC;
}

inline bool isRestartDetectedWIRE(  Sercom *sercom )
{
	return sercom->I2CS.STATUS.bit.SR;
}

inline bool isAddressMatch(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.AMATCH;
}

inline bool isMasterReadOperationWIRE(  Sercom *sercom )
{
	return sercom->I2CS.STATUS.bit.DIR;
}

inline bool isRXNackReceivedWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.RXNACK;
}

int availableWIRE(  Sercom *sercom );

void prepareNackBitWIRE( Sercom *sercom );

void prepareAckBitWIRE( Sercom *sercom );

void prepareCommandBitsWire(Sercom *sercom, uint8_t cmd);

uint8_t readDataWIRE(  Sercom *sercom );

void initClock( Sercom *sercom );

void resetWIRE(Sercom *sercom);

void enableWIRE(Sercom *sercom);

inline void disableInterruptsWIRE( Sercom * sercom )
{
	// clear the interrupt register
  sercom->I2CS.INTENCLR.reg = SERCOM_I2CS_INTENCLR_PREC |   // Stop
                              SERCOM_I2CS_INTENCLR_AMATCH | // Address Match
                              SERCOM_I2CS_INTENCLR_DRDY ;   // Data Ready
}

inline void enableInterruptsWIRE( Sercom * sercom )
{
	// Set the interrupt register
  sercom->I2CS.INTENSET.reg = SERCOM_I2CS_INTENSET_PREC |   // Stop
                              SERCOM_I2CS_INTENSET_AMATCH | // Address Match
                              SERCOM_I2CS_INTENSET_DRDY ;   // Data Ready
}

inline void disableWIRE(Sercom *sercom)
{
	disableInterruptsWIRE( sercom );
	// I2C Master and Slave modes share the ENABLE bit function.

	// Enable the I²C master mode
	sercom->I2CM.CTRLA.bit.ENABLE = 0 ;

	while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 )
	{
		// Waiting the enable bit from SYNCBUSY is equal to 0;
	}
}

void initSlaveWIRE( Sercom *sercom, uint8_t ucAddress );

void resetUART( Sercom * sercom );

bool isDataRegisterEmptyUART( Sercom * sercom );

void initUART( Sercom * sercom, SercomUartSampleRate sampleRate, uint32_t baudrate);

void initFrame( Sercom * sercom , SercomUartCharSize charSize, SercomDataOrder dataOrder, SercomParityMode parityMode, SercomNumberStopBit nbStopBits);

void initPads( Sercom * sercom , SercomUartTXPad txPad, SercomRXPad rxPad);

void enableUART( Sercom * sercom );

void disableUART( Sercom *sercom );

void setUARTBaud( Sercom *sercom, uint32_t baudrate );

inline void flushUART( Sercom * sercom )
{
  // Skip checking transmission completion if data register is empty
  if(isDataRegisterEmptyUART(sercom))
    return;

  // Wait for transmission to complete
  while(!sercom->USART.INTFLAG.bit.TXC);
}

inline void clearStatusUART( Sercom * sercom )
{
  //Reset (with 0) the STATUS register
  sercom->USART.STATUS.reg = SERCOM_USART_STATUS_RESETVALUE;
}

inline bool availableDataUART( Sercom * sercom )
{
  //RXC : Receive Complete
  return sercom->USART.INTFLAG.bit.RXC;
}

inline bool isUARTError( Sercom * sercom )
{
  return sercom->USART.INTFLAG.bit.ERROR;
}

inline void acknowledgeUARTError( Sercom * sercom )
{
  sercom->USART.INTFLAG.bit.ERROR = 1;
}

inline bool isBufferOverflowErrorUART( Sercom * sercom )
{
  //BUFOVF : Buffer Overflow
  return sercom->USART.STATUS.bit.BUFOVF;
}

inline bool isFrameErrorUART( Sercom * sercom )
{
  //FERR : Frame Error
  return sercom->USART.STATUS.bit.FERR;
}

inline bool isParityErrorUART( Sercom * sercom )
{
  //PERR : Parity Error
  return sercom->USART.STATUS.bit.PERR;
}

inline uint8_t readDataUART( Sercom * sercom )
{
  return sercom->USART.DATA.bit.DATA;
}

inline bool isEnabledUART( Sercom * sercom )
{
	return sercom->USART.CTRLA.bit.ENABLE;
}

int writeDataUART( Sercom * sercom ,uint8_t data);
int writeDataUART( Sercom * sercom , char const *buffer);

void initSPI( Sercom * sercom, SercomSpiTXPad mosi, SercomRXPad miso, SercomSpiCharSize charSize, SercomDataOrder dataOrder) ;
void initSPISlave( Sercom *sercom, SercomSpiTXPad miso, SercomRXPad mosi, SercomSpiCharSize charSize, SercomDataOrder dataOrder) ;
void initSPIClock( Sercom * sercom, SercomSpiClockMode clockMode, uint32_t baudrate) ;

void resetSPI( Sercom * sercom ) ;
void enableSPI( Sercom * sercom ) ;
void disableSPI( Sercom * sercom ) ;
void setDataOrderSPI( Sercom * sercom, SercomDataOrder dataOrder) ;
SercomDataOrder getDataOrderSPI( Sercom * sercom ) ;
void setBaudrateSPI( Sercom * sercom, uint8_t divider) ;
void setClockModeSPI( Sercom * sercom, SercomSpiClockMode clockMode) ;
uint8_t transferDataSPI( Sercom * sercom, uint8_t data) ;
bool isBufferOverflowErrorSPI( Sercom * sercom ) ;
bool isDataRegisterEmptySPI( Sercom * sercom ) ;
bool isTransmitCompleteSPI( Sercom * sercom ) ;
bool isReceiveCompleteSPI( Sercom * sercom ) ;

#endif
