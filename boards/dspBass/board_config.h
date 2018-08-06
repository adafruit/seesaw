#ifndef SEESAW_DEVICE_CONFIG_H
#define SEESAW_DEVICE_CONFIG_H

#define PRODUCT_CODE 0x1235
#define CONFIG_NO_EEPROM
#define USB_UART_DIRECT
#define BFIN_HWRST_PIN 3

//* ============== POOL SIZES =================== *//
#define EVT_SIZE_SMALL 32
#define EVT_SIZE_MEDIUM 64
#define EVT_SIZE_LARGE 256
#define EVT_COUNT_SMALL 128
#define EVT_COUNT_MEDIUM 32
#define EVT_COUNT_LARGE 16

//* ============== ADC =================== *//
#define CONFIG_ADC 0
#define CONFIG_ADC_INPUT_0 1

#define CONFIG_ADC_INPUT_1 1

#define CONFIG_ADC_INPUT_2 1

#define CONFIG_ADC_INPUT_3 0

//* ============== DAC =================== *//
#define CONFIG_DAC 0

//* ============== TIMER =================== *//
#define CONFIG_TIMER 0

#define CONFIG_TIMER_PWM_OUT0 1

#define CONFIG_TIMER_PWM_OUT1 1

#define CONFIG_TIMER_PWM_OUT2 0

#define CONFIG_TIMER_PWM_OUT3 0

//* ============== INTERRUPT =================== *//
#define CONFIG_INTERRUPT 0
#define CONFIG_INTERRUPT_PIN 22

//* ============== I2C SLAVE =================== *//
#define CONFIG_I2C_SLAVE 1
#define CONFIG_I2C_SLAVE_SERCOM SERCOM4
#define CONFIG_I2C_SLAVE_HANDLER SERCOM4_Handler
#define CONFIG_I2C_SLAVE_IRQn SERCOM4_IRQn
#define CONFIG_I2C_SLAVE_PIN_SDA (32 + 8)
#define CONFIG_I2C_SLAVE_PIN_SCL (32 + 9)
#define CONFIG_I2C_SLAVE_MUX 3

//* ============== SPI SLAVE =================== *//
#define CONFIG_SPI_SLAVE 0

//* ============== SERCOM =================== *//
#define CONFIG_SERCOM0 1
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

#define CONFIG_NEOPIXEL_BUF_MAX 64

//* =========== PEDAL ================ *//
#define CONFIG_PEDAL 0
#define CONFIG_PEDAL_SERCOM SERCOM2
#define CONFIG_PEDAL_HANDLER SERCOM2_Handler
#define CONFIG_PEDAL_IRQn SERCOM2_IRQn

#define CONFIG_PEDAL_PIN_MOSI 12
#define CONFIG_PEDAL_PIN_MISO 13
#define CONFIG_PEDAL_PIN_SCK 15
#define CONFIG_PEDAL_PIN_SS 14
#define CONFIG_PEDAL_MUX 2

#define CONFIG_PEDAL_PAD_TX SPI_PAD_0_SCK_3
#define CONFIG_PEDAL_PAD_RX SERCOM_RX_PAD_1

#define CONFIG_PEDAL_CHAR_SIZE SPI_CHAR_SIZE_8_BITS
#define CONFIG_PEDAL_DATA_ORDER MSB_FIRST
#define CONFIG_PEDAL_DATA_MODE SERCOM_SPI_MODE_0
#define CONFIG_PEDAL_BAUD_RATE 4000000

#define CONFIG_PEDAL_A0_PIN 2
#define CONFIG_PEDAL_A0_CHANNEL 0

#define CONFIG_PEDAL_A1_PIN 5
#define CONFIG_PEDAL_A1_CHANNEL 5

#define CONFIG_PEDAL_A2_PIN 6
#define CONFIG_PEDAL_A2_CHANNEL 6

#define CONFIG_PEDAL_A3_PIN 7
#define CONFIG_PEDAL_A3_CHANNEL 7

#define CONFIG_PEDAL_A4_PIN 4
#define CONFIG_PEDAL_A4_CHANNEL 4

#define CONFIG_PEDAL_A5_PIN 8
#define CONFIG_PEDAL_A5_CHANNEL 16

#define CONFIG_PEDAL_RELAY_SET_PIN 0
#define CONFIG_PEDAL_RELAY_RST_PIN 1

#define CONFIG_PEDAL_BTN_PIN 20

#define CONFIG_PEDAL_FS1_PIN 21
#define CONFIG_PEDAL_FS2_PIN 23

#define CONFIG_PEDAL_START_PIN 13

#define CONFIG_PEDAL_SYNC_INTERVAL 17

#define CONFIG_PEDAL_DMAC_CHANNEL 2
#define CONFIG_PEDAL_DMAC_TRIGGER SERCOM2_DMAC_ID_TX

//* =========== USB ================ *//
#define CONFIG_USB 1

#define USB_VID 0x239A
#define USB_PID 0x001B

#define USB_PRODUCT "DSP Feather"

#endif
