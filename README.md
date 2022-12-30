# gate-opener

Implements HomeKit GarageDoor for Homebridge. For use with `homebridge-mqttthing`.

## Setup

```
cp src/config.example.h src/config.h
```

Adjust config as needed in `src/config.h`.

## Hardware

Designed for use with ESP8266, specifically Sonoff SV.

### Flashing Sonoff SV

1. Using FTDI programmer, connect to header pins on the Sonoff SV:

    - GND - GND
    - TX  - RX
    - RX  - TX
    - VCC - 3v3

2. Hold down the Sonoff button while connecting USB to computer
3. Upload sketch in Arduino editor

### Flashing ESP32 DOIT DevKit

1. Plugin USB
2. In Arduino editor press Upload
3. When "Connecting..." hold down BOOT button until flashing begins

## License

MIT. See LICENSE.txt.
