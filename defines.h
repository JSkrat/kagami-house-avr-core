/*
 * defines.h
 *
 * Created: 02.01.2019 20:21:14
 *  Author: J-sama
 */ 


#ifndef DEFINES_INTERNAL_H_
#define DEFINES_INTERNAL_H_

//#define CEILING(x,y) (((x) + (y) - 1) / (y))
//#define STR2(s) #s
//#define STR(s) STR2(s)

#define BT_MASTER 10
#define BT_SLAVE 20
#define BT_DEBUG 30

/*** transciever ***/
#define MAC_SIZE 5
#define PAYLOAD_SIZE 32
#define SLAVE_RESPONSE_TIMEOUT_MS 20
#define SLAVE_REQUEST_TIMEOUT_MS 500

// re-define that with code to run in case of packet damaging error, like spi communication error or wrong response length
// if error should lead to program halt, it should be implemented in that macro
// codes are:
// 0 for incorrect message length in nrf24l01.c/nRF24L01_read_received_data <- this happened once on yellow, so recover from that
// 1 for incorrect response length in RF protocol.c/generateResponse
// 2 for incorrect payload length in nRF model.c/nRF_transmit
// 3 for RFBUFFER overflow in RF model master.c/dataReceived
// 4 for RFBUFFER overflow in RF model master.c/dataTransmitted
// 5 for RFBUFFER overflow in RF model master.c/transmissionFailed
// 6 for RFBUFFER overflow in RF model master.c/responseTimeoutEvent
// 7 for oversized payload size in RF model.c/RFTransmit
#define RF_ERROR(code) while (1);

/*enum eInternalErrorCode {
	eie
};*/

// re-define that with code to run when enter and exit all RF functions
// for example to blink led when packet received
#define RF_FUNCTION(enter, unit, function)

/*** USART ***/
#define BAUDRATE 200000

#include "../project defines.h"

#endif /* DEFINES_INTERNAL_H_ */