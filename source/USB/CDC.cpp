/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <sam.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "USB/USBCore.h"
#include "USB/USBAPI.h"
#include "USB/USBDesc.h"
#include "bsp.h"

#include "AOUSB.h"
#include "SeesawConfig.h"

#ifdef CDC_ENABLED

#define CDC_SERIAL_BUFFER_SIZE	256

/* For information purpose only since RTS is not always handled by the terminal application */
#define CDC_LINESTATE_DTR		0x01 // Data Terminal Ready
#define CDC_LINESTATE_RTS		0x02 // Ready to Send

#define CDC_LINESTATE_READY		(CDC_LINESTATE_RTS | CDC_LINESTATE_DTR)

typedef struct {
	uint32_t dwDTERate;
	uint8_t bCharFormat;
	uint8_t bParityType;
	uint8_t bDataBits;
	uint8_t lineState;
} LineInfo;

_Pragma("pack(1)")
static volatile LineInfo _usbLineInfo = {
	115200, // dWDTERate
	0x00,   // bCharFormat
	0x00,   // bParityType
	0x08,   // bDataBits
	0x00    // lineState
};

static volatile int32_t breakValue = -1;

static CDCDescriptor _cdcInterface = {
	D_IAD(0, 2, CDC_COMMUNICATION_INTERFACE_CLASS, CDC_ABSTRACT_CONTROL_MODEL, 0),

	// CDC communication interface
	D_INTERFACE(CDC_ACM_INTERFACE, 1, CDC_COMMUNICATION_INTERFACE_CLASS, CDC_ABSTRACT_CONTROL_MODEL, 0),
	D_CDCCS(CDC_HEADER, CDC_V1_10 & 0xFF, (CDC_V1_10>>8) & 0x0FF), // Header (1.10 bcd)

	D_CDCCS4(CDC_ABSTRACT_CONTROL_MANAGEMENT, 6), // SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE supported
	D_CDCCS(CDC_UNION, CDC_ACM_INTERFACE, CDC_DATA_INTERFACE), // Communication interface is master, data interface is slave 0
	D_CDCCS(CDC_CALL_MANAGEMENT, 1, 1), // Device handles call management (not)
	D_ENDPOINT(USB_ENDPOINT_IN(CDC_ENDPOINT_ACM), USB_ENDPOINT_TYPE_INTERRUPT, 0x10, 0x10),

	// CDC data interface
	D_INTERFACE(CDC_DATA_INTERFACE, 2, CDC_DATA_INTERFACE_CLASS, 0, 0),
	D_ENDPOINT(USB_ENDPOINT_OUT(CDC_ENDPOINT_OUT), USB_ENDPOINT_TYPE_BULK, EPX_SIZE, 0),
	D_ENDPOINT(USB_ENDPOINT_IN (CDC_ENDPOINT_IN ), USB_ENDPOINT_TYPE_BULK, EPX_SIZE, 0)
};
_Pragma("pack()")

const void* _CDC_GetInterface(void)
{
	return &_cdcInterface;
}

uint32_t _CDC_GetInterfaceLength(void)
{
	return sizeof(_cdcInterface);
}

int CDC_GetInterface(uint8_t* interfaceNum)
{
	interfaceNum[0] += 2;	// uses 2
	return USBDevice.sendControl(&_cdcInterface,sizeof(_cdcInterface));
}

bool CDC_Setup(USBSetup& setup)
{
	uint8_t requestType = setup.bmRequestType;
	uint8_t r = setup.bRequest;

	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (r == CDC_GET_LINE_CODING)
		{
			USBDevice.sendControl((void*)&_usbLineInfo, 7);
			return true;
		}
	}

	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (r == CDC_SET_LINE_CODING)
		{
			USBDevice.recvControl((void*)&_usbLineInfo, 7);
#if defined(USB_UART_DMA) || defined(USB_UART_DIRECT)
			AOUSB::setBaudRate(_usbLineInfo.dwDTERate);
#endif
		}

		if (r == CDC_SET_CONTROL_LINE_STATE)
		{
			_usbLineInfo.lineState = setup.wValueL;
		}

		if (r == CDC_SET_LINE_CODING || r == CDC_SET_CONTROL_LINE_STATE)
		{
			// auto-reset into the bootloader is triggered when the port, already
			// open at 1200 bps, is closed. We check DTR state to determine if host 
			// port is open (bit 0 of lineState).
			if (_usbLineInfo.dwDTERate == 1200 && (_usbLineInfo.lineState & 0x01) == 0)
			{
				initiateReset(250);
			}
			else
			{
				cancelReset();
			}
			return false;
		}

		if (CDC_SEND_BREAK == r)
		{
			breakValue = ((uint16_t)setup.wValueH << 8) | setup.wValueL;
			return false;
		}
	}
	return false;
}

#endif
