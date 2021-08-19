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
#include "../KagamiCore/RF protocol internal.h"
#include "../KagamiCore/RF info.h"
//#include "../KagamiCore/RF custom functions.h"


//static enum eResponseCodes lastSentPacketStatus;


enum eResponseCodes validatePacket(const uint8_t length, const sRequest *data) {
	// first lets check the packet minimum length
	if (offsetof(sRequest, rqData) > length) return ercBadRequestData;
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

fRFFunction findFunctionByCode(uint8_t unit, eU0Functions code) {
	uint8_t count;
	const tRFCodeFunctionItem *list;
	if (0x10 > code) {
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
		const uint8_t iCode = pgm_read_byte(&(list[i].functionCode));
		if (iCode == code) {
			fRFFunction iFunction = pgm_read_ptr(&(list[i].function));
			return iFunction;
		}
	}
	return NULL;
}

void generateResponse(const uint8_t requestLength, const uint8_t *requestData, uint8_t *responseLength, uint8_t *responseData) {
	// response should be already allocated for that function
	#define REQUEST_DATA ((const sRequest*) requestData)
	#define RESPONSE_DATA ((sResponse*) responseData)
	enum eResponseCodes validation = validatePacket(requestLength, REQUEST_DATA);
	RESPONSE_DATA->rsVersion = PROTOCOL_VERSION;
	RESPONSE_DATA->rsTransactionId = REQUEST_DATA->rqTransactionId;
    RESPONSE_DATA->rsCode = (uint8_t) validation;
	*responseLength = 3; // length of the empty response, without any data
	switch (validation) {
		case ercOk: {
            fRFFunction method = findFunctionByCode(REQUEST_DATA->rqUnitId, REQUEST_DATA->rqFunctionId);
			if (NULL == method) {
				RESPONSE_DATA->rsCode = ercBadFunctionId;
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
			RESPONSE_DATA->rsCode = (*method)(
				REQUEST_DATA->rqUnitId,
				REQUEST_DATA->rqFunctionId,
				&requestArg,
				&responseArg
			);
			*responseLength += responseArg.length;
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
}

void generateAdvertisement(uint8_t *packetLength, uint8_t *packetData) {
	#define DATA ((sResponse*) packetData)
	fRFFunction method = findFunctionByCode(0, eFGetProperties);
	sString responseArg = {
		.length = 0,
		.data = &(DATA->rsData[0])
	};
	DATA->rsVersion = PROTOCOL_VERSION;
	DATA->rsTransactionId = 0xAA;
	DATA->rsCode = (*method)(0, eFGetProperties, NULL, &responseArg);
	*packetLength = 3 + responseArg.length;
	#undef DATA
}

void protocolInit() {
	lastTransacrionId = 0;
	RFCustomFunctionsInit();
}

#endif
