#ifndef BSP_SERCOM_H
#define BSP_SERCOM_H

#include "sam.h"

#define I2C_SLAVE_OPERATION 0x4u
#define I2C_MASTER_OPERATION 0x5u

#define GCM_SERCOMx_SLOW          (0x13U)
#define GCM_SERCOM0_CORE          (0x14U)
#define GCM_SERCOM1_CORE          (0x15U)
#define GCM_SERCOM2_CORE          (0x16U)
#define GCM_SERCOM3_CORE          (0x17U)
#define GCM_SERCOM4_CORE          (0x18U)
#define GCM_SERCOM5_CORE          (0x19U)

enum
{
	WIRE_UNKNOWN_STATE = 0x0ul,
	WIRE_IDLE_STATE,
	WIRE_OWNER_STATE,
	WIRE_BUSY_STATE
};

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

bool isMasterWIRE(  Sercom *sercom )
{
	return sercom->I2CS.CTRLA.bit.MODE == I2C_MASTER_OPERATION;
}

bool isSlaveWIRE(  Sercom *sercom )
{
	return sercom->I2CS.CTRLA.bit.MODE == I2C_SLAVE_OPERATION;
}

bool isBusIdleWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_IDLE_STATE;
}

bool isBusOwnerWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_OWNER_STATE;
}

bool isDataReadyWIRE(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.DRDY;
}

bool isStopDetectedWIRE(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.PREC;
}

bool isRestartDetectedWIRE(  Sercom *sercom )
{
	return sercom->I2CS.STATUS.bit.SR;
}

bool isAddressMatch(  Sercom *sercom )
{
	return sercom->I2CS.INTFLAG.bit.AMATCH;
}

bool isMasterReadOperationWIRE(  Sercom *sercom )
{
	return sercom->I2CS.STATUS.bit.DIR;
}

bool isRXNackReceivedWIRE(  Sercom *sercom )
{
	return sercom->I2CM.STATUS.bit.RXNACK;
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
		clockId = GCM_SERCOM0_CORE;
	}
	else if(sercom == SERCOM1)
	{
		clockId = GCM_SERCOM1_CORE;
	}
	else if(sercom == SERCOM2)
	{
		clockId = GCM_SERCOM2_CORE;
	}
	else if(sercom == SERCOM3)
	{
		clockId = GCM_SERCOM3_CORE;
	}
	else if(sercom == SERCOM4)
	{
		clockId = GCM_SERCOM4_CORE;
	}
	else if(sercom == SERCOM5)
	{
		clockId = GCM_SERCOM5_CORE;
	}

	//Setting clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( clockId ) | // Generic Clock 0 (SERCOMx)
	GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
	GCLK_CLKCTRL_CLKEN ;
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_SERCOMx_SLOW ) |
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
  // I2C Master and Slave modes share the ENABLE bit function.

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

void disableWIRE(Sercom *sercom)
{
  // I2C Master and Slave modes share the ENABLE bit function.

  // Enable the I²C master mode
  sercom->I2CM.CTRLA.bit.ENABLE = 0 ;

  while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 )
  {
    // Waiting the enable bit from SYNCBUSY is equal to 0;
  }
}

void initSlaveWIRE( Sercom *sercom, uint8_t ucAddress )
{
  // Initialize the peripheral clock and interruption
  initClock(sercom) ;
  resetWIRE(sercom) ;

  // Set slave mode
  sercom->I2CS.CTRLA.bit.MODE = I2C_SLAVE_OPERATION ;
  
  sercom->I2CS.CTRLB.bit.AMODE = 0x00;

  sercom->I2CS.ADDR.reg = SERCOM_I2CS_ADDR_ADDR( ucAddress & 0x7Ful ) | // 0x7F, select only 7 bits
                          SERCOM_I2CS_ADDR_ADDRMASK( 0x00ul ) ;         // 0x00, only match exact address

  // Set the interrupt register
  sercom->I2CS.INTENSET.reg = SERCOM_I2CS_INTENSET_PREC |   // Stop
                              SERCOM_I2CS_INTENSET_AMATCH | // Address Match
                              SERCOM_I2CS_INTENSET_DRDY ;   // Data Ready

  while ( sercom->I2CM.SYNCBUSY.bit.SYSOP != 0 )
  {
    // Wait the SYSOP bit from SYNCBUSY to come back to 0
  }
}

#endif