/*
 * Settings.h
 *
 * Created: 16.05.2020 20:27:36
 *  Author: Mintytail
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>
#include "../project settings.h"

uint8_t readSetting(eSetting type, void *output);
uint8_t saveSetting(eSetting type, const void *data);

#endif /* SETTINGS_H_ */