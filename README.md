# kagami-house-avr-core
Core communication module for avr microcontrollers

# Usage

You need to run `rf_init` function from `RF model.h` and `functions_init` from `RF functions.h` before enabling interrupts.

If you need uart communication, you need to run `u_init` from `UART parser.h` as well.

# Build

Copy `project_defines example.h` and `settings example.ini` removing 'example' part from filename to your project root directory. Edit accordingly to your settings.

## Build number

You need to run `build number.py` with full path to `version.h` before build to update version number (it is current timestamp in uint32).

## Settings (EEPROM map)

You need to run `KagamiCore/settings map.py "settings.ini" "project_settings.h" "project_settings.c"`