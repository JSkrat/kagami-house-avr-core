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

/*** USART ***/
#define BAUDRATE 200000

#include "../project defines.h"

#endif /* DEFINES_INTERNAL_H_ */