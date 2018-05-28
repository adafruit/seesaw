/*
 * main.cpp
 *
 * Created: 7/17/2017 3:26:42 PM
 * Author : deanm
 */ 


#include "sam.h"
#include "board_init.h"

#include "qpcpp.h"
#include "qp_extras.h"

#include "event.h"
#include "bsp.h"
#include "bsp_gpio.h"
#include "bsp_sercom.h"
#include "bsp_nvmctrl.h"

#include "SeesawConfig.h"

#include "System.h"
#include "Delegate.h"
#include "I2CSlave.h"
#include "SPISlave.h"
#include "AOADC.h"
#include "AODAC.h"
#include "AOTimer.h"
#include "AOInterrupt.h"
#include "AOSERCOM.h"
#include "AODAP.h"
#include "AOTouch.h"
#include "Neopixel.h"
#include "AOUSB.h"
#include "AOPedal.h"

#include "bsp_gpio.h"

using namespace QP;

uint32_t evtPoolSmall[ROUND_UP_DIV_4(EVT_SIZE_SMALL * EVT_COUNT_SMALL)];
uint32_t evtPoolMedium[ROUND_UP_DIV_4(EVT_SIZE_MEDIUM * EVT_COUNT_MEDIUM)];
uint32_t evtPoolLarge[ROUND_UP_DIV_4(EVT_SIZE_LARGE * EVT_COUNT_LARGE)];
QP::QSubscrList subscrSto[MAX_PUB_SIG];

static System sys;
static Delegate del;

#if CONFIG_I2C_SLAVE
static I2CSlave i2c( CONFIG_I2C_SLAVE_SERCOM );
#endif

#if CONFIG_SPI_SLAVE
static SPISlave spi( CONFIG_SPI_SLAVE_SERCOM );
#endif

#if CONFIG_DAC
static AODAC dac;
#endif

#if CONFIG_ADC
static AOADC adc;
#endif

#if CONFIG_TIMER
static AOTimer tmr;
#endif

#if CONFIG_TOUCH
static AOTouch touch;
#endif

#if CONFIG_INTERRUPT
static AOInterrupt interrupt;
#endif

#if CONFIG_SERCOM0
static AOSERCOM sercom0( SERCOM0, AO_SERCOM0, 0 );
#endif

#if CONFIG_SERCOM1
static AOSERCOM sercom1( SERCOM1, AO_SERCOM1, 1 );
#endif

#if CONFIG_SERCOM2
static AOSERCOM sercom2( SERCOM2, AO_SERCOM2, 2 );
#endif

#if CONFIG_SERCOM5
static AOSERCOM sercom5( SERCOM5, AO_SERCOM5, 5 );
#endif

#if CONFIG_DAP
static AODAP dap;
#endif

#if CONFIG_NEOPIXEL
static Neopixel neopixel;
#endif

#if CONFIG_USB
static AOUSB usb;
#endif

#if CONFIG_PEDAL
static AOPedal pedal( CONFIG_PEDAL_SERCOM) ;
#endif

static inline void doNothing(int delay)
{
	while (delay > 0){
		asm("nop");
		--delay;
	}
}

static inline bool spiRdy()
{
	return (gpio_read_bulk() & (1ul << 22)) == 0;
}

static void bootBfin()
{
		//TODO: this is used for SPI MISO
		pinPeripheral(16, 0);
	    gpio_dirclr_bulk(PORTA, 1UL << 16);
		gpio_pullenclr_bulk(1UL << 16);
		gpio_outclr_bulk(PORTA, 1UL << 16);

		//dsp !reset
		gpio_init(PORTA, BFIN_HWRST_PIN, 1);
		
#ifdef CORE_CLKOUT
		//DSP FEATHER SPECIFIC: start clock output
		pinPeripheral(BFIN_CLK_PIN, 7);
#endif
		
		//pulse reset
		doNothing(100ul);
		gpio_write(PORTA, BFIN_HWRST_PIN, 0);
		doNothing(100ul);
		gpio_write(PORTA, BFIN_HWRST_PIN, 1);
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	board_init();
	
	QF::init();
	QF::poolInit(evtPoolSmall, sizeof(evtPoolSmall), EVT_SIZE_SMALL);
	QF::poolInit(evtPoolMedium, sizeof(evtPoolMedium), EVT_SIZE_MEDIUM);
	QF::poolInit(evtPoolLarge, sizeof(evtPoolLarge), EVT_SIZE_LARGE);
	QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe
	
	BspInit();
	
	bootBfin();

	//Start active objects.
	sys.Start(PRIO_SYSTEM);
	del.Start(PRIO_DELEGATE);
	
#if CONFIG_I2C_SLAVE
	i2c.Start(PRIO_I2C_SLAVE);
#endif

#if CONFIG_SPI_SLAVE
	spi.Start(PRIO_SPI_SLAVE);
#endif
	
#if CONFIG_ADC
	adc.Start(PRIO_ADC);
#endif

#if CONFIG_DAC
	dac.Start(PRIO_DAC);
#endif

#if CONFIG_TIMER
	tmr.Start(PRIO_TIMER);
#endif

#if CONFIG_TOUCH
    touch.Start(PRIO_TOUCH);
#endif

#if CONFIG_INTERRUPT
	interrupt.Start(PRIO_INTERRUPT);
#endif

#if CONFIG_SERCOM0
	sercom0.Start(PRIO_SERCOM);
#endif

#if CONFIG_SERCOM1
	sercom1.Start(PRIO_SERCOM);
#endif

#if CONFIG_SERCOM2
	sercom2.Start(PRIO_SERCOM);
#endif

#if CONFIG_SERCOM5
	sercom5.Start(PRIO_SERCOM);
#endif

#if CONFIG_DAP
	dap.Start(PRIO_DAP);
#endif

#if CONFIG_NEOPIXEL
	neopixel.Start(PRIO_NEOPIXEL);
#endif

#if CONFIG_USB
	usb.Start(PRIO_USB);
#endif

#if CONFIG_PEDAL
	pedal.Start(PRIO_PEDAL);
#endif
	
	//publish a start request
	Evt *evt = new SystemStartReq(0);
	QF::PUBLISH(evt, dummy);
	
	QP::QF::run();
}
