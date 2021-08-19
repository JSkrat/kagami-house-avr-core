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

const static uint16_t settingsSize[esAmount] = {
	[esKey] = 32,
	[esAddress] = MAC_SIZE,
	[esChannel] = 1,
	[esMode] = 1,
	[esU0Description] = 24,
	[esU1Description] = 24,
};

const static uint16_t settingsAddress[esAmount] = {
	[esKey] = 16,
	[esAddress] = 8,
	[esChannel] = 0,
	[esMode] = 1,
	[esU0Description] = 48,
	[esU1Description] = 72,
	// next free 96
};

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

