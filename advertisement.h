/*
 * advertisement.h
 *
 * Created: 18.08.2021 20:14:32
 *  Author: Mintytail
 */ 

/*
to use advertisement, define those:
#define USE_ADVERTISEMENT
#define ADV_BUTTON_DDR DDRD
#define ADV_BUTTON_PIN PIND6
#define ADV_BUTTON_PORT PORTD
#define ADV_LED_DDR DDRD
#define ADV_LED_PIN PORTD7
#define ADV_LED_PORT PORTD
*/

#ifndef ADVERTISEMENT_H_
#define ADVERTISEMENT_H_

void advertisement_init();
void advertisement_process();

#endif /* ADVERTISEMENT_H_ */
