# kagami-house-avr-core
Core communication module for avr microcontrollers

# Usage

You need to run `rf_init` function from `RF model.h` and `functions_init` from `RF functions.h` before enabling interrupts.

If you need uart communication, you need to run `u_init` from `UART parser.h` as well.

# Build

Project tree is following:

- this submodule in KagamiCore directory
- project files may reside in own directory
- configuration files `project defines.h` and `settings.ini` should reside in project root, `../` relative to this submodule files

Copy `project defines example.h` and `settings example.ini` removing 'example' part from filename to your project root directory. Edit accordingly to your settings.


## Build number

You need to run `build number.py "KagamiCode/version.h"` before build to update version number (it is current timestamp in uint32).

## Settings (EEPROM map)

You need to run `"KagamiCore/settings map.py" "settings.ini" "project settings.h" "project settings.c"`

# Hardware

## nrf module connections

NRF24L01 module should be connected to port B:

- IRQ to PB0
- CSN to PB1
- CE to PB2
- rest of spi to spi on port B (PB3, PB4, PB5)
