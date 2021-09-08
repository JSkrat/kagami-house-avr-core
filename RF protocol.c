/*
 * protocol.c
 *
 * Created: 01.03.2020 22:24:02
 *  Author: Mintytail
 */ 
#if BT_SLAVE == BUILD_TYPE

#include "../KagamiCore/RF protocol.h"
#include "../KagamiCore/RF functions.h"
#ifndef UNIT_TESTING
	#include <avr/pgmspace.h>
#else
    #include "../usb2nrf_tests/pgmspace.h"
#endif
#include <stdint.h>
#include <string.h> // for NULL
#include <stddef.h> // for offsetof
#include <stdbool.h>
#include "../KagamiCore/RF protocol internal.h"
#include "../KagamiCore/RF info.h"
#include "defines.h"
//#include "../KagamiCore/RF custom functions.h"


//static enum eResponseCodes lastSentPacketStatus;


enum eResponseCodes validatePacket(const uint8_t length, const sRequest *data) {
	// first lets check the packet minimum length
	if (offsetof(sRequest, rqData) > length) return ercBadRequestData;
	if (PAYLOAD_SIZE < length) return ercBadRequestData;
	// check the contents
	if (PROTOCOL_VERSION != data->rqVersion) return ercBadVersion;
	/*if (lastTransacrionId+1 != data->rqTransactionId) {
		// do not check transaction id if it is reset transaction id
		if (eFResetTransactionId != data->rqFunctionId)
			return ercNotConsecutiveTransactionId;
	}*/
	// check if unit exists
	if (unitsCount+1 <= data->rqUnitId) {
		return ercBadUnitId;
	}
	//if (_eFCount <= data->rqFunctionId) return ercBadFunctionId;
	return ercOk;
}

enum eResponseCodes validateNodeArguments(const fNodeType nodeType, const uint8_t argumentsLength, const uint8_t const *arguments) {
	(void) arguments;
	if (0 == argumentsLength) {
		if (nodeType.readable) return ercOk;
		else return ercNodePermissionViolation;
	} else {
		if (! nodeType.writable) return ercNodePermissionViolation;
		switch (nodeType.dataType) {
			case edtBool: if (1 != argumentsLength) return ercBadArguments; break;
			case edtByte: if (1 != argumentsLength) return ercBadArguments; break;
			case edtInt32: if (4 != argumentsLength) return ercBadArguments; break;
			default: break;
		}
		return ercOk;
	}
};

enum eResponseCodes validateMethodArguments(const fMethodType methodType, const uint8_t argumentsLength, const uint8_t const *arguments) {
	(void) arguments;
	if (0 == argumentsLength) {
		return ercOk;
	} else {
		switch (methodType.input) {
			case edtBool: if (1 != argumentsLength) return ercBadArguments; break;
			case edtByte: if (1 != argumentsLength) return ercBadArguments; break;
			case edtInt32: if (4 != argumentsLength) return ercBadArguments; break;
			default: break;
		}
		return ercOk;
	}
}

tRFCodeFunctionItem findFunctionByCode(uint8_t unit, fDataID code) {
	uint8_t count;
	const tRFCodeFunctionItem *list;
	tRFCodeFunctionItem ret;
	memset(&ret, 0, sizeof(tRFCodeFunctionItem));
	if (0x10 > code.data.dataId) {
		// search in common functions
		count = fUCount;
		list = RFStandardFunctions;
	} else {
		// search individually
		if (0 == unit) {
			count = fU0Count;
			list = RFU0Functions;
		} else {
			count = pgm_read_byte(&(RFUnits[unit-1].length));
			list = pgm_read_ptr(&(RFUnits[unit-1].functions));
		}
	}
	for (uint8_t i = 0; i < count; i++) {
		char iCode = pgm_read_byte(&(list[i].dataId));
		if (iCode == code.byte) {
			ret.dataId.byte = iCode;
			ret.function = pgm_read_ptr(&(list[i].function));
			ret.type.byte = pgm_read_byte(&(list[i].type));
			break;
		}
	}
	return ret;
}

void generateResponse(const uint8_t requestLength, const uint8_t *requestData, uint8_t *responseLength, uint8_t *responseData) {
	RF_FUNCTION(true, 0, 0);
	// response should be already allocated for that function
	#define REQUEST_DATA ((const sRequest*) requestData)
	#define RESPONSE_DATA ((sResponse*) responseData)
	enum eResponseCodes validation = validatePacket(requestLength, REQUEST_DATA);
	RESPONSE_DATA->rsVersion = RESPONSE_PROTOCOL_VERSION;
	RESPONSE_DATA->rsTransactionId = REQUEST_DATA->rqTransactionId;
    RESPONSE_DATA->rsCode = (uint8_t) validation;
	*responseLength = RESPONSE_HEADER_SIZE; // length of the empty response, without any data
	switch (validation) {
		case ercOk: {
            tRFCodeFunctionItem methodItem = findFunctionByCode(REQUEST_DATA->rqUnitId, REQUEST_DATA->rqFunctionId.rqFunctionId);
			if (NULL == methodItem.function) {
				RESPONSE_DATA->rsCode = ercBadFunctionId;
				*responseLength += 2;
				RESPONSE_DATA->rsData[0] = REQUEST_DATA->rqUnitId;
				RESPONSE_DATA->rsData[1] = REQUEST_DATA->rqFunctionId.byte;
				break;
			}
			const scString requestArg = {
				.length = requestLength - offsetof(sRequest, rqData),
				.data = &(REQUEST_DATA->rqData[0])
			};
			sString responseArg = {
				.length = 0,
				.data = &(RESPONSE_DATA->rsData[0])
			};
			// validate arguments
			if (ediNode == methodItem.dataId.data.type) {
				RESPONSE_DATA->rsCode = validateNodeArguments(methodItem.type.nodeType, requestArg.length, requestArg.data);
			} else { //if (ediMethod == methodItem.dataId.type) {
				RESPONSE_DATA->rsCode = validateMethodArguments(methodItem.type.methodType, requestArg.length, requestArg.data);
			}
			if (ercOk != RESPONSE_DATA->rsCode) break;
			// run the function \0/
			RESPONSE_DATA->rsCode = (*(methodItem.function))(
				REQUEST_DATA->rqUnitId,
				REQUEST_DATA->rqFunctionId.byte,
				&requestArg,
				&responseArg
			);
			*responseLength += responseArg.length;
			if (PAYLOAD_SIZE < *responseLength) {
				RF_ERROR(1);
				// in case error handler does not halt
				*responseLength = 1;
				RESPONSE_DATA->rsCode = ercInternalError;
				RESPONSE_DATA->rsData[0] = 1;
			}
			// update statistics
            if (0x80 > RESPONSE_DATA->rsCode) ok_responses++;
			else error_responses++;
			break;
		}
		// this is responsible for transaction integrity
		// we will keep hash of the previous request and its response
		// and if the new one is the same plus tid is current-1
		// there will be repeat of the response, but tid will be new one anyway
		// it can be repeated predefined number of tries, every time increasing tid
		// after which we shall stop that and respond some special error code
		/*case ercNotConsecutiveTransactionId: {
			missed_packets++;
			RESPONSE_DATA->rsData[0] = lastTransacrionId;
		}*/
		default: {
			validation_errors++;
			break;
		}
	}
	#undef RESPONSE_DATA
	#undef REQUEST_DATA
	RF_FUNCTION(false, 0, 0);
}

void generateAdvertisement(uint8_t *packetLength, uint8_t *packetData) {
	// advertisement is like a response to a function 0, unit 0, but with the protocol version
	#define DATA ((sResponse*) packetData)
	fDataID id;
	id.data.type = ediNode; id.data.dataId = eFProperties;
	tRFCodeFunctionItem code = findFunctionByCode(0, id);
	fRFFunction method = code.function;
	sString responseArg = {
		.length = 0,
		.data = &(DATA->rsData[0])
	};
	DATA->rsVersion = RESPONSE_PROTOCOL_VERSION;
	DATA->rsTransactionId = 0xAA;
	DATA->rsCode = (*method)(0, eFProperties, NULL, &responseArg);
	*packetLength = 3 + responseArg.length;
	#undef DATA
}

void protocolInit() {
	lastTransacrionId = 0;
}

#endif
