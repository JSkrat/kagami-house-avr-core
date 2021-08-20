/*
 * Settings.c
 *
 * Created: 16.05.2020 20:27:23
 *  Author: Mintytail
 */ 
#include "Settings.h"
#include "defines.h"
#include <avr/eeprom.h>
#include <stdint.h>
#include "../project_settings.c"

uint8_t readSetting(eSetting type, void *output) {
	if (esAmount <= type) return 0;
	eeprom_read_block(output, (void*) settingsAddress[type], settingsSize[type]);
	return settingsSize[type];
}

uint8_t saveSetting(eSetting type, const void *data) {
	if (esAmount <= type) return 0;
	eeprom_update_block(data, (void*) settingsAddress[type], settingsSize[type]);
	return settingsSize[type];
}

