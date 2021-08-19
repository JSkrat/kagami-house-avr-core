/*
 * UART_functions.h
 *
 * Created: 13.04.2020 22:08:33
 *  Author: Mintytail
 */ 


#ifndef UART_FUNCTIONS_H_
#define UART_FUNCTIONS_H_

#include "sstring.h"
#ifndef UNIT_TESTING
    #include <avr/pgmspace.h>
#else
    #include "../usb2nrf_tests/pgmspace.h"
#endif
#include <stddef.h>


typedef uint8_t (*fUARTFunction)(const scString*, sString*);

// modem commands
typedef enum {
	mcEcho = 0,
	mcVersion = 1,
	mcStatus = 8,
	mcAddresses = 9,
	
	mcSetChannel = 0x10,
	mcSetTXPower = 0x11,
	mcSetBitRate = 0x12,
	mcSetAutoRetransmitDelay = 0x13,
	mcSetAutoRetransmitCount = 0x14,
	
	mcClearTX = 0x20,
	mcClearRX = 0x21,
	
	mcListen = 0x30,
	
	mcSetMode = 0x40,
	mcSetListenAddress = 0x41,
	
	mcReadRFBuffer = 0x50,
	
	mcTransmit = 0x7F,

    mcResponseFlag = 0x80,
    // for unit testing, should be the number of absent function
    mcNoFunction = 0x70,
} eModemCommand;


typedef struct {
	eModemCommand command;
	fUARTFunction function;
} tUARTCommandItem;


#define UART_FUNCTIONS_NUMBER 16
extern const tUARTCommandItem UARTFunctions[UART_FUNCTIONS_NUMBER];

#endif /* UART_FUNCTIONS_H_ */
