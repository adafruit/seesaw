#include "bsp_adc.h"

#define ADC_TEMP_SAMPLE_LENGTH 4
#define INT1V_VALUE_FLOAT 1.0
#define INT1V_DIVIDER_1000 1000.0
#define ADC_10BIT_FULL_SCALE_VALUE_FLOAT 1023.0

static uint32_t tempR;       // Production Room temperature
static uint32_t tempH;       // Production Hot temperature

static int32_t VADCR;       // Room temperature ADC voltage
static int32_t VADCH;       // Hot temperature ADC voltage

static uint32_t INT1VR;      // Room temp 2's complement of the internal 1V reference value
static uint32_t INT1VH;      // Hot temp 2's complement of the internal 1V reference value

static void adc_set_defaults()
{
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 |    // Divide Clock by 64.
					ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default

	ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length

	while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;   // No Negative input (Internal Ground)

	// Averaging (see datasheet table in AVGCTRL register description)
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |    // 1 sample only (no oversampling nor averaging)
						ADC_AVGCTRL_ADJRES(0x0ul);   // Adjusting result by 0

	ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA = 0.5* 3V3 = 1.65V
}

void adc_init()
{
	// Initialize Analog Controller
	// Setting clock
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_ADC_Val ) | // Generic Clock ADC
						GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
						GCLK_CLKCTRL_CLKEN ;
						
	while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains
	
	ADC->CTRLA.bit.SWRST = 1;
	
	while( ADC->STATUS.bit.SYNCBUSY == 1 || ADC->CTRLA.bit.SWRST == 1);

	adc_set_defaults();
}

void adc_set_freerunning(bool mode)
{
	syncADC();
	ADC->CTRLA.bit.ENABLE = 0;
	
	syncADC();
	ADC->CTRLB.bit.FREERUN = mode;
	
	syncADC();
	ADC->CTRLA.bit.ENABLE = mode;
	
	syncADC();
}

void adc_set_inputscan(uint8_t channels)
{
	syncADC();
	ADC->INPUTCTRL.bit.INPUTSCAN = channels;
}

uint16_t adc_read(uint8_t channel)
{
	syncADC();
	ADC->INPUTCTRL.bit.MUXPOS = channel; // Selection for the positive ADC input

	// Control A
	/*
	* Bit 1 ENABLE: Enable
	*   0: The ADC is disabled.
	*   1: The ADC is enabled.
	* Due to synchronization, there is a delay from writing CTRLA.ENABLE until the peripheral is enabled/disabled. The
	* value written to CTRL.ENABLE will read back immediately and the Synchronization Busy bit in the Status register
	* (STATUS.SYNCBUSY) will be set. STATUS.SYNCBUSY will be cleared when the operation is complete.
	*
	* Before enabling the ADC, the asynchronous clock source must be selected and enabled, and the ADC reference must be
	* configured. The first conversion after the reference is changed must not be used.
	*/
	syncADC();
	ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC

	// Start conversion
	syncADC();
	ADC->SWTRIG.bit.START = 1;

	// Clear the Data Ready flag
	ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

	// Start conversion again, since The first conversion after the reference is changed must not be used.
	syncADC();
	ADC->SWTRIG.bit.START = 1;

	// Store the value
	while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
	uint16_t valueRead = ADC->RESULT.reg;

	syncADC();
	ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
	syncADC();
	
	return valueRead;
}

void init_temp() {
	volatile uint32_t val1;    /* Temperature Log Row Content first 32 bits */
    volatile uint32_t val2;    /* Temperature Log Row Content another 32 bits */

	uint8_t room_temp_val_int; /* Integer part of room temperature in °C */
	uint8_t room_temp_val_dec; /* Decimal part of room temperature in °C */
	uint8_t hot_temp_val_int;  /* Integer part of hot temperature in °C */
	uint8_t hot_temp_val_dec;  /* Decimal part of hot temperature in °C */
	int8_t room_int1v_val;     /* internal 1V reference drift at room temperature */
	int8_t hot_int1v_val;      /* internal 1V reference drift at hot temperature*/

	uint16_t ADCR;     // Production Room temperature ADC value
	uint16_t ADCH;     // Production Hot temperature ADC value

    uint32_t *temp_log_row_ptr = (uint32_t *)NVMCTRL_TEMP_LOG;

    val1 = *temp_log_row_ptr;
    temp_log_row_ptr++;
    val2 = *temp_log_row_ptr;

    room_temp_val_int = (uint8_t)((val1 & FUSES_ROOM_TEMP_VAL_INT_Msk) >> FUSES_ROOM_TEMP_VAL_INT_Pos);
    room_temp_val_dec = (uint8_t)((val1 & FUSES_ROOM_TEMP_VAL_DEC_Msk) >> FUSES_ROOM_TEMP_VAL_DEC_Pos);

    hot_temp_val_int = (uint8_t)((val1 & FUSES_HOT_TEMP_VAL_INT_Msk) >> FUSES_HOT_TEMP_VAL_INT_Pos);
    hot_temp_val_dec = (uint8_t)((val1 & FUSES_HOT_TEMP_VAL_DEC_Msk) >> FUSES_HOT_TEMP_VAL_DEC_Pos);

    room_int1v_val = (int8_t)((val1 & FUSES_ROOM_INT1V_VAL_Msk) >> FUSES_ROOM_INT1V_VAL_Pos);
    hot_int1v_val = (int8_t)((val2 & FUSES_HOT_INT1V_VAL_Msk) >> FUSES_HOT_INT1V_VAL_Pos);

    ADCR = (uint16_t)((val2 & FUSES_ROOM_ADC_VAL_Msk) >> FUSES_ROOM_ADC_VAL_Pos);
    ADCH = (uint16_t)((val2 & FUSES_HOT_ADC_VAL_Msk) >> FUSES_HOT_ADC_VAL_Pos);

    tempR = ((uint32_t)room_temp_val_int << 16) + ((1UL << 16)/10 * room_temp_val_dec); //this is gonna be decimal val 1-10?
    tempH = ((uint32_t)hot_temp_val_int << 16) + ((1UL << 16)/10 * hot_temp_val_dec);

    INT1VR = ((1UL << 16) - 1) - (int16_t)room_int1v_val * 66; //1/1000 V step
    INT1VH = ((1UL << 16) - 1) - (int16_t)hot_int1v_val * 66;

    VADCR = ((((uint64_t)ADCR << 4) * (uint64_t)INT1VR) >> 16);
    VADCH = ((((uint64_t)ADCH << 4) * (uint64_t)INT1VH) >> 16);
}

int32_t calculate_temperature()
{

    int32_t VADC;      /* Voltage calculation using ADC result for Coarse Temp calculation */
    //uint32_t VADCM;     /* Voltage calculation using ADC result for Fine Temp calculation. */
    //uint32_t INT1VM;    /* Voltage calculation for reality INT1V value during the ADC conversion */

	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 |    // Divide Clock by 32.
					ADC_CTRLB_RESSEL_12BIT;         // 12 bits resolution

	ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length

	while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;   // No Negative input (Internal Ground)
	ADC->INPUTCTRL.bit.GAIN = 0;

	// Averaging (see datasheet table in AVGCTRL register description)
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_4 | 
						ADC_AVGCTRL_ADJRES(0);

	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INT1V_Val;

	SYSCTRL->VREF.bit.TSEN = 1;

	//read the ADC channel
	uint32_t raw_value = adc_read(ADC_INPUTCTRL_MUXPOS_TEMP_Val);

    VADC = (raw_value << 4);

    // calculate fine temperature using Equation1 and Equation
    // 1b as mentioned in data sheet section "Temperature Sensor Characteristics"
    // of Electrical Characteristics. (adapted from ASF sample code).
    // Coarse Temp Calculation by assume INT1V=1V for this ADC conversion

	int64_t numerator = ( ((int64_t)(VADC - VADCR) * (int64_t)(tempH - tempR)) >> 16 );

    int32_t coarse_temp = tempR + ( ( numerator << 16 ) / (VADCH - VADCR) );

    return coarse_temp;

#if 0
    // Calculation to find the real INT1V value during the ADC conversion
    numerator = ( ((int64_t)(INT1VH - INT1VR) * (int64_t)(coarse_temp - tempR)) >> 16);
    INT1VM = INT1VR +  ( ( numerator << 16 ) / (tempH - tempR));

	VADCM = ((int64_t)VADC * INT1VM) >> 16;

    // Fine Temp Calculation by replace INT1V=1V by INT1V = INT1Vm for ADC conversion
	numerator = ( ((int64_t)(VADCM - VADCR) * (int64_t)(tempH - tempR)) >> 16 );
	int32_t fine_temp = tempR + ( ( numerator << 16 ) / (VADCH - VADCR) );

	adc_set_defaults();

	return fine_temp;
#endif
}
