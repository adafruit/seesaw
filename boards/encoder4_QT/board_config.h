#ifndef SEESAW_DEVICE_CONFIG_H
#define SEESAW_DEVICE_CONFIG_H

#define PRODUCT_CODE 0
#define CONFIG_NO_ACTIVITY_LED
#define CONFIG_NO_EEPROM

//* ============== POOL SIZES =================== *//
#define	EVT_SIZE_SMALL 16
#define EVT_SIZE_MEDIUM 32
#define	EVT_SIZE_LARGE 64
#define	EVT_COUNT_SMALL 16
#define	EVT_COUNT_MEDIUM 8
#define	EVT_COUNT_LARGE 1

//* ============== ADC =================== *//
#define CONFIG_ADC 0
#define CONFIG_ADC_INPUT_0 0
#define CONFIG_ADC_INPUT_1 0
#define CONFIG_ADC_INPUT_2 0
#define CONFIG_ADC_INPUT_3 0
//* ============== DAC =================== *//
#define CONFIG_DAC 0

//* ============== TIMER =================== *//
#define CONFIG_TIMER 0
#define CONFIG_TIMER_PWM_OUT0 0
#define CONFIG_TIMER_PWM_OUT1 0
#define CONFIG_TIMER_PWM_OUT2 0
#define CONFIG_TIMER_PWM_OUT3 0

//* ============== INTERRUPT =================== *//
#define CONFIG_INTERRUPT 1
#define CONFIG_INTERRUPT_PIN 25

//* ============== I2C SLAVE =================== *//
#define CONFIG_I2C_SLAVE 1
#define PIN_ADDR_0 5
#define PIN_ADDR_1 6
#define CONFIG_I2C_SLAVE_ADDR 0x36

//* ============== ENCODER =================== *//
#define CONFIG_ENCODER 1
#define CONFIG_NUM_ENCODERS 4
#define CONFIG_ENCODER0_A_PIN 8
#define CONFIG_ENCODER0_B_PIN 9
#define CONFIG_ENCODER1_A_PIN 10
#define CONFIG_ENCODER1_B_PIN 11
#define CONFIG_ENCODER2_A_PIN 2
#define CONFIG_ENCODER2_B_PIN 3
#define CONFIG_ENCODER3_A_PIN 4
#define CONFIG_ENCODER3_B_PIN 5

//* ============== SERCOM =================== *//
#define CONFIG_SERCOM0 0
#define CONFIG_SERCOM1 0
#define CONFIG_SERCOM2 0

//These are only available on samd21
#define CONFIG_SERCOM3 0
#define CONFIG_SERCOM4 0
#define CONFIG_SERCOM5 0

//* ============== DAP =================== *//
#define CONFIG_DAP 0

//* =========== NEOPIXEL ================ *//
#define CONFIG_NEOPIXEL 1
 #define CONFIG_NEOPIXEL_BUF_MAX 64
#endif
