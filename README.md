
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
* `python` in path (optional)
* `arm-none-eabi-gcc` and `arm-none-eabi-g++` in the path

### Build commands

The default board is `debug`. You can build a different one using:

```
make BOARD=samd09breakout
```

# Contributing

Contributions are welcome! Please read our [Code of Conduct](https://github.com/adafruit/seesaw/blob/master/CODE_OF_CONDUCT.md) before contributing to help this project stay welcoming.
