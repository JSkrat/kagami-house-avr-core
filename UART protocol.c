/*
 * UART_protocol.c
 *
 * Created: 19.04.2020 20:57:19
 *  Author: Mintytail
 */ 
#if BT_MASTER == BUILD_TYPE || BT_DEBUG == BUILD_TYPE

#include "UART protocol.h"
#include "UART functions.h"
#include "sstring.h"
#include <stddef.h>
#include <stdbool.h>

// packet prefixes
#define uPPProtoVer 0

typedef enum {
	usError,
	usProtoVer,
	usCommand,
	usPDataLength,
	usPData,
	usEnd,
} usState;

static usState state;
static uint8_t position = 0;
static uPackage reqBuffer;


void UARTGenerateErrorResponse(const eUARTResponseCodes code, uPackage *response) {
    response->pkg.protocolVersion = uPPProtoVer;
    response->pkg.command = mcNoFunction | mcResponseFlag;
    response->pkg.payloadSize = 0;
    response->pkg.code = code;
}

void UARTGenerateResponse(const uPackage *request, uPackage *response) {
	response->pkg.protocolVersion = uPPProtoVer;
    response->pkg.command = request->pkg.command | mcResponseFlag;
	response->pkg.payloadSize = 0;
	if (uPPProtoVer != request->pkg.protocolVersion) {
		response->pkg.code = eucBadVersion;
		return;
	}
	// search for the requested command
	for (uint8_t i = 0; i < UART_FUNCTIONS_NUMBER; i++) {
        eModemCommand commandToCheck = pgm_read_byte((const uint8_t*) &(UARTFunctions[i].command));
		if (request->pkg.command == commandToCheck) {
            fUARTFunction f = (fUARTFunction) pgm_read_ptr(&(UARTFunctions[i].function));
			if (NULL == f) {
				// generate response "not implemented"
				response->pkg.code = eucNotImplemented;
			} else {
				scString sUARTRequest = {.length = request->pkg.payloadSize, .data = &request->pkg.payload[0]};
				sString sUARTResponse = {.length = 0, .data = &response->pkg.payload[0]};
				response->pkg.code = f(&sUARTRequest, &sUARTResponse);
				response->pkg.payloadSize = sUARTResponse.length;
			}
			return;
		}
	}
	// command not found, generate error response about it
	response->pkg.code = eucBadCommand;
}

void UARTBeginTransaction() {
	state = usProtoVer;
}

bool UARTProcessNextByte(uint8_t b, uPackage *response) {
	switch (state) {
        case usEnd: {
            break;
        }
		case usError: {
            break;
		}
		case usProtoVer: {
			if (uPPProtoVer == b) {
				state = usCommand;
            } else {
				state = usError;
                UARTGenerateErrorResponse(eucBadVersion, response);
                return true;
			}
			break;
		}
		case usCommand: {
			if (0x80 & b) {
				state = usError;
                UARTGenerateErrorResponse(eucBadCommand, response);
                return true;
            } else {
				reqBuffer.pkg.command = b;
                state = usPDataLength;
			}
			break;
		}
		case usPDataLength: {
			if (PAYLOAD_SIZE + MAC_SIZE <= b) {
				state = usError;
                UARTGenerateErrorResponse(eucMemoryError, response);
                return true;
			}
			reqBuffer.pkg.payloadSize = b;
			position = 0;
			if (0 == b) {
				// there won't be any bytes from that package anymore
				state = usError;
				UARTGenerateResponse(&reqBuffer, response);
				return true;
			} else {
				state = usPData;
			}
			break;
		}
		case usPData: {
			reqBuffer.pkg.payload[position++] = b;
			if (reqBuffer.pkg.payloadSize <= position) {
				// that's all payload, no bytes from that package left
                state = usEnd;
				UARTGenerateResponse(&reqBuffer, response);
				return true;
			}
			break;
		}
	}
	return false;
}

void UARTProtocolInit() {
	state = usError;
}

#endif