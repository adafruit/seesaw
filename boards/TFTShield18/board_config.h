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
#define CONFIG_TIMER 1
#define CONFIG_TIMER_PWM_OUT0 1
#define CONFIG_TIMER_PWM_OUT0_PIN 4
#define CONFIG_TIMER_PWM_OUT0_TC TC1
#define CONFIG_TIMER_PWM_OUT0_WO 0
#define CONFIG_TIMER_PWM_OUT1 0
#define CONFIG_TIMER_PWM_OUT2 0
#define CONFIG_TIMER_PWM_OUT3 0

//* ============== INTERRUPT =================== *//
#define CONFIG_INTERRUPT 0
#define CONFIG_INTERRUPT_PIN 0

//* ============== I2C SLAVE =================== *//
#define CONFIG_I2C_SLAVE 1
#define CONFIG_I2C_SLAVE_SERCOM SERCOM1
#define CONFIG_I2C_SLAVE_HANDLER SERCOM1_Handler
#define CONFIG_I2C_SLAVE_IRQn SERCOM1_IRQn
#define CONFIG_I2C_SLAVE_PIN_SDA 22
#define CONFIG_I2C_SLAVE_PIN_SCL 23
#define CONFIG_I2C_SLAVE_MUX 2
#define CONFIG_I2C_SLAVE_FLOW_CONTROL 0
#define CONFIG_I2C_SLAVE_FLOW_CONTROL_PIN 0
#define CONFIG_I2C_SLAVE_ADDR 0x2E

//* ============== SERCOM =================== *//
#define CONFIG_SERCOM0 0
#define CONFIG_SERCOM1 0
#define CONFIG_SERCOM1 0
#define CONFIG_SERCOM2 0

//These are only available on samd21
#define CONFIG_SERCOM3 0
#define CONFIG_SERCOM4 0
#define CONFIG_SERCOM5 0

//* ============== DAP =================== *//
#define CONFIG_DAP 0

//* =========== NEOPIXEL ================ *//
#define CONFIG_NEOPIXEL 0
 #define CONFIG_NEOPIXEL_BUF_MAX 0
#endif
