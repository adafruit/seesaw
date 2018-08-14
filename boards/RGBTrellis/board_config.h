#ifndef SEESAW_DEVICE_CONFIG_H
#define SEESAW_DEVICE_CONFIG_H

#define PRODUCT_CODE 0000

//override default I2C addr
#define CONFIG_I2C_SLAVE_ADDR 0x2E

#define CONFIG_NO_ACTIVITY_LED
#define CONFIG_NO_EEPROM

#define CONFIG_ADDR_2 1
#define CONFIG_ADDR_3 1
#define CONFIG_ADDR_4 1
#define PIN_ADDR_0 17
#define PIN_ADDR_1 16
#define PIN_ADDR_2 15
#define PIN_ADDR_3 14
#define PIN_ADDR_4 11

//* ============== POOL SIZES =================== *//
#define	EVT_SIZE_SMALL 16
#define EVT_SIZE_MEDIUM 32
#define	EVT_SIZE_LARGE 64
#define	EVT_COUNT_SMALL 16
#define	EVT_COUNT_MEDIUM 8
#define	EVT_COUNT_LARGE 1

//* ============== ADC =================== *//
#define CONFIG_ADC 0
#define CONFIG_ADC_INPUT_0 1

#define CONFIG_ADC_INPUT_1 1

#define CONFIG_ADC_INPUT_2 0

#define CONFIG_ADC_INPUT_3 0

//* ============== DAC =================== *//
#define CONFIG_DAC 0

//* ============== TIMER =================== *//
#define CONFIG_TIMER 0

//override default PWM0 pin
#define CONFIG_TIMER_PWM_OUT0 1

#define CONFIG_TIMER_PWM_OUT0_TC TC1
#define CONFIG_TIMER_PWM_OUT0_WO 1
#define CONFIG_TIMER_PWM_OUT0_PIN 5

#define CONFIG_TIMER_PWM_OUT1 0

#define CONFIG_TIMER_PWM_OUT2 0

#define CONFIG_TIMER_PWM_OUT3 0

//* ============== INTERRUPT =================== *//
#define CONFIG_INTERRUPT 1
#define CONFIG_INTERRUPT_PIN 27

#define CONFIG_INTERRUPT_OPEN_DRAIN 1

//* ============== I2C SLAVE =================== *//
#define CONFIG_I2C_SLAVE 1

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

#define CONFIG_NEOPIXEL_BUF_MAX (16*4)

//* =========== KEYPAD ================ *//
#define CONFIG_KEYPAD 1

#define CONFIG_KEYPAD_ROW0 1
#define CONFIG_KEYPAD_ROW1 1
#define CONFIG_KEYPAD_ROW2 1
#define CONFIG_KEYPAD_ROW3 1

#define CONFIG_KEYPAD_COL0 1
#define CONFIG_KEYPAD_COL1 1
#define CONFIG_KEYPAD_COL2 1
#define CONFIG_KEYPAD_COL3 1

#define CONFIG_KEYPAD_ROW0_PIN 4
#define CONFIG_KEYPAD_ROW1_PIN 5
#define CONFIG_KEYPAD_ROW2_PIN 9
#define CONFIG_KEYPAD_ROW3_PIN 10

#define CONFIG_KEYPAD_COL0_PIN 2
#define CONFIG_KEYPAD_COL1_PIN 6
#define CONFIG_KEYPAD_COL2_PIN 7
#define CONFIG_KEYPAD_COL3_PIN 8

#endif