/*
 * i2c_qp.cpp
 *
 * Created: 7/17/2017 3:26:42 PM
 * Author : deanm
 */ 


#include "sam.h"

#include "qpcpp.h"
#include "qp_extras.h"

#include "event.h"
#include "bsp.h"

#include "System.h"
#include "Delegate.h"
#include "I2CSlave.h"
#include "AOADC.h"
#include "AODAC.h"
#include "AOGPIO.h"

using namespace QP;

enum {
	EVT_SIZE_SMALL = 32,
	EVT_SIZE_MEDIUM = 64,
	EVT_SIZE_LARGE = 256,
	EVT_COUNT_SMALL = 32,
	EVT_COUNT_MEDIUM = 4,
	EVT_COUNT_LARGE = 1,
};
uint32_t evtPoolSmall[ROUND_UP_DIV_4(EVT_SIZE_SMALL * EVT_COUNT_SMALL)];
uint32_t evtPoolMedium[ROUND_UP_DIV_4(EVT_SIZE_MEDIUM * EVT_COUNT_MEDIUM)];
uint32_t evtPoolLarge[ROUND_UP_DIV_4(EVT_SIZE_LARGE * EVT_COUNT_LARGE)];
QP::QSubscrList subscrSto[MAX_PUB_SIG];

static System sys;
static Delegate del;
static I2CSlave i2c;
static AODAC dac;
static AOADC adc;
static AOGPIO gpio;

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	
	QF::init();
	QF::poolInit(evtPoolSmall, sizeof(evtPoolSmall), EVT_SIZE_SMALL);
	QF::poolInit(evtPoolMedium, sizeof(evtPoolMedium), EVT_SIZE_MEDIUM);
	QF::poolInit(evtPoolLarge, sizeof(evtPoolLarge), EVT_SIZE_LARGE);
	QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe
	
	BspInit();
	//Start active objects.
	sys.Start(PRIO_SYSTEM);
	del.Start(PRIO_DELEGATE);
	i2c.Start(PRIO_I2C_SLAVE);
	adc.Start(PRIO_ADC);
	dac.Start(PRIO_DAC);
	gpio.Start(PRIO_GPIO);
	
	//publish a start request
	Evt *evt = new SystemStartReq(0);
	QF::PUBLISH(evt, dummy);
	
	QP::QF::run();
}