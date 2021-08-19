/*
 * UART_protocol.h
 *
 * Created: 14.04.2020 11:09:23
 *  Author: Mintytail
 */ 


#ifndef UART_PROTOCOL_H_
#define UART_PROTOCOL_H_
#include "defines.h"
#include <stdint.h>
#include <stdbool.h>
#include "../KagamiCore/UART functions.h"

typedef enum {
	eucOk = 0,
	eucNoPackets = 0x10,
	eucSlaveResponseTimeout = 0x11,
	eucAckTimeout = 0x12,
	eucDataPacket = 0x14,
	eucAckPacket = 0x15,
	eucGeneralFail = 0x80,
	eucBadVersion = 0x90,
	eucBadCommand = 0x91,
	eucMemoryError = 0x92,
	eucArgumentValidationError = 0x93,
	eucNotImplemented = 0x94,
} eUARTResponseCodes;

// packets from uart
//#define HEADER_SIZE (3)
#define RESPONSE_HEADER_SIZE (4)
typedef union {
	uint8_t packageBuffer[(RESPONSE_HEADER_SIZE + PAYLOAD_SIZE)];
	struct {
		uint8_t protocolVersion;
		eModemCommand command;
        eUARTResponseCodes code;
		uint8_t payloadSize;
		uint8_t payload[PAYLOAD_SIZE];
	} pkg;
} uPackage;

void UARTProtocolInit();
void UARTBeginTransaction();
bool UARTProcessNextByte(uint8_t b, uPackage *response);

#endif /* UART_PROTOCOL_H_ */
