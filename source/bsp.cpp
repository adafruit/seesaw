///***************************************************************************
// Product: DPP example, STM32 NUCLEO-L152RE board, preemptive QK kernel
// Last updated for version 5.6.5
// Last updated on  2016-07-05
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************

//#include <string.h>
#include "qpcpp.h"
#include "bsp.h"
#include "bsp_nvmctrl.h"
#include "sam.h"
#include "bsp_gpio.h"
#include "bsp_sercom.h"

#include "SeesawConfig.h"
#include "Delegate.h"
#include <stdlib.h>

//Q_DEFINE_THIS_FILE

#define ENABLE_BSP_PRINT

volatile uint32_t lastGPIOState = 0;
volatile uint32_t _systemMs = 0;

void operator delete(void * p)
{
  free(p);
}

void BspInit() {
	//initialize some clocks
#if defined(__SAMD21G18A__)
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0 | PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM5 ;
	PM->APBCMASK.reg |= PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 ;
#else
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0 | PM_APBCMASK_SERCOM1 | PM_APBCMASK_TC1 | PM_APBCMASK_TC2;
#endif

#ifdef TCC0
	PM->APBCMASK.reg |= PM_APBCMASK_TCC0 ;
#endif

#ifdef SERCOM2
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM2 ;
#endif

#ifdef DAC
	PM->APBCMASK.reg |= PM_APBCMASK_ADC | PM_APBCMASK_DAC ;
#endif

	eeprom_init();

/*
	GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC_Val));
	// Enable EIC
	EIC->CTRL.bit.ENABLE = 1;
	while (EIC->STATUS.bit.SYNCBUSY == 1) { }
		*/

#ifdef ENABLE_LOGGING
    pinPeripheral(CONFIG_LOG_UART_PIN_TX, CONFIG_LOG_UART_PIN_TX_MUX);

    initUART(CONFIG_LOG_SERCOM, SAMPLE_RATE_x16, CONFIG_LOG_UART_BAUD_RATE);
    initFrame(CONFIG_LOG_SERCOM, CONFIG_LOG_UART_CHAR_SIZE, LSB_FIRST, CONFIG_LOG_UART_PARITY, CONFIG_LOG_UART_STOP_BIT);
    initPads(CONFIG_LOG_SERCOM, CONFIG_LOG_UART_PAD_TX, CONFIG_LOG_UART_PAD_RX);

    enableUART(CONFIG_LOG_SERCOM);
#endif
}

void BspWrite(char const *buf, uint32_t len) {
	//TODO:
}

uint32_t GetSystemMs() {
    return _systemMs;
}

extern "C" {
	void SysTick_Handler(void) {
		QXK_ISR_ENTRY();
		QP::QF::tickX_(0);
		_systemMs++;
		
		//process GPIO interrupts
		uint32_t GPIOState = gpio_read_bulk();
		if( (Delegate::m_inten & GPIOState) != (Delegate::m_inten & lastGPIOState) ){
			Delegate::m_intflag |= GPIOState ^ lastGPIOState;
			Delegate::intCallback();
		}
		lastGPIOState = GPIOState;
		
		QXK_ISR_EXIT();
	}
}

// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
	
    // assigning all priority bits for preemption-prio. and none to sub-prio.
    //NVIC_SetPriorityGrouping(0U);
	
	NVIC_SetPriority(PendSV_IRQn, 0xFF);
	SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
	NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO);
	NVIC_SetPriority(CONFIG_I2C_SLAVE_IRQn, I2C_SLAVE_ISR_PRIO);
	//NVIC_SetPriority(NVMCTRL_IRQn, NVMCTRL_ISR_PRIO);

#if defined(SERCOM0)
	NVIC_SetPriority(SERCOM0_IRQn, SERCOM_ISR_PRIO);
#endif

#if defined(SERCOM1)
	NVIC_SetPriority(SERCOM1_IRQn, SERCOM_ISR_PRIO);
#endif

#if defined(SERCOM2)
	NVIC_SetPriority(SERCOM2_IRQn, SERCOM_ISR_PRIO);
#endif

#if defined(SERCOM5)
	NVIC_SetPriority(SERCOM5_IRQn, SERCOM_ISR_PRIO);
#endif
    
    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //NVIC_SetPriority(EXTI0_1_IRQn,   DPP::EXTI0_1_PRIO);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(SysTick_IRQn);
	//NVIC_EnableIRQ(NVMCTRL_IRQn);
#if CONFIG_I2C_SLAVE
	NVIC_EnableIRQ(CONFIG_I2C_SLAVE_IRQn);
#endif

#if CONFIG_SERCOM0
	NVIC_EnableIRQ(SERCOM0_IRQn);
#endif

#if CONFIG_SERCOM1
	NVIC_EnableIRQ(SERCOM1_IRQn);
#endif

#if CONFIG_SERCOM2
	NVIC_EnableIRQ(SERCOM2_IRQn);
#endif

#if CONFIG_SERCOM5
	NVIC_EnableIRQ(SERCOM5_IRQn);
#endif
}

//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QXK::onIdle(void) {
    // toggle the User LED on and then off (not enough LEDs, see NOTE01)
    //QF_INT_DISABLE();

    //QF_INT_ENABLE();


#if defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    // !!!CAUTION!!!
    // The WFI instruction stops the CPU clock, which unfortunately disables
    // the JTAG port, so the ST-Link debugger can no longer connect to the
    // board. For that reason, the call to __WFI() has to be used with CAUTION.
    //
    // NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    // reset the board, then connect with ST-Link Utilities and erase the part.
    // The trick with BOOT(0) is it gets the part to run the System Loader
    // instead of your broken code. When done disconnect BOOT0, and start over.
    //
    //__WFI();   Wait-For-Interrupt
#endif
}

//............................................................................
extern "C" void Q_onAssert(char const * const module, int loc) {
	//
    // NOTE: add here your application-specific error handling
    //

    QF_INT_DISABLE();
	__BKPT();
	while(1);
}

extern "C" void assert_failed(char const *module, int loc) {
	__BKPT();
	while(1);
}


} // namespace QP

