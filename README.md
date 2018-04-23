
# Introduction

Seesaw is an open source microcontroller friend for other chips. It provides a
variety of capabilities such as UART, ADC, DAC, extra GPIO, etc. to chips that don't have them.

# Interfacing
- [Arduino](https://github.com/adafruit/Adafruit_Seesaw)
- [CircuitPython](https://github.com/adafruit/Adafruit_CircuitPython_seesaw)
- [Python](https://github.com/adafruit/Adafruit_Python_seesaw)

## Build

### Requirements

* `make` and a Unix environment
* `python` in path
* `arm-none-eabi-gcc` and `arm-none-eabi-g++` in the path

### Build commands

The default board is `debug`. You can build a different one using:

```
make BOARD=samd09breakout
```

add `DEBUG=1` to the `make` command to enable UART debug logging. The default UART TX pin is `PA04` and the default baud rate is `115200`. Note that there may not be enough available flash to enable UART debug loggnig on smaller chips such as the SAMD09. Also note that enabling UART debug logging may prevent the I2C slave interface from operating at full speed.

# Contributing

Contributions are welcome! Please read our [Code of Conduct](https://github.com/adafruit/seesaw/blob/master/CODE_OF_CONDUCT.md) before contributing to help this project stay welcoming.
