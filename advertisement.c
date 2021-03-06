/*
 * advertisement.c
 *
 * Created: 18.08.2021 20:14:32
 *  Author: Mintytail
 */ 

#include "advertisement.h"
#include "defines.h"
#include "RF model.h"
#include <avr/io.h>

#ifdef USE_ADVERTISEMENT
static int adv = 0;
// anti-rattle
#define ADV_TRIGGER 2

void advertisement_init() {
	ADV_BUTTON_DDR &= ~_BV(ADV_BUTTON_PIN);
	ADV_LED_DDR |= _BV(ADV_LED_PIN);
	adv = 0;
}

/// call this continuously in a main loop between sleeps
void advertisement_process() {
	// check the button
	if (ADV_TRIGGER > adv) {
		if (ADV_BUTTON_PORT & _BV(ADV_BUTTON_PIN)) adv++;
		else adv = 0;
	} else {
		switchMode(emSearchMaster);
	}
	// show the state on led
	if (emSearchMaster == getRFMode()) {
		ADV_LED_PORT |= _BV(ADV_LED_PIN);
	} else {
		ADV_LED_PORT &= ~_BV(ADV_LED_PIN);
	}
}
#endif
