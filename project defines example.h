
// RF mode
#define BUILD_TYPE BT_SLAVE
// advertisement
#define ADV_BUTTON_PORT PIND
#define ADV_BUTTON_DDR DDRD
#define ADV_BUTTON_PIN PIND0
#define ADV_LED_PORT PORTC
#define ADV_LED_DDR DDRC
#define ADV_LED_PIN PORTC2
// transciever
#define portTransiever PORTB
#define PORT_IN_TRANSIEVER PINB
#define poTransiever_IRQ PORTB0
// 8-bit timer number to use inside KagamiCore
#define RF_TIMER 2
/*** USART (redefine in case it is used and has different rate) ***/
//#define BAUDRATE 200000
