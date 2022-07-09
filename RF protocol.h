/*
 * protocol.h
 *
 * Created: 01.03.2020 22:24:15
 *  Author: Mintytail
 *
 * Main protocol file. Provides interface request-response for slave
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdbool.h>
#include <stdint.h>
#include "sstring.h"
#if defined(UNIT_TESTING) || defined(NOT_FIRMWARE)
    #pragma pack(push)
    #pragma pack(1)
#endif

#define PROTOCOL_VERSION 1
#define RESPONSE_PROTOCOL_VERSION 2

// arguments are request and response
// caller should allocate space for the response
// unit (first argument) is guaranteed (by RF Parser) to be correct
typedef uint8_t (*fRFFunction)(const uint8_t, const uint8_t, const scString*, sString*);

#if defined(UNIT_TESTING) || defined(NOT_FIRMWARE)
	typedef enum: uint8_t {
#else
	typedef enum {
#endif
		ediNode = 0,
		ediMethod = 1
	} eDataIdType;

#define fDataID_type_mask 0x80
#define fDataID_type_offset 7
#define fDataID_dataId_mask 0x7F
#define fDataID_dataId_offset 0
typedef union {
	struct {
		eDataIdType type: 1;
		uint8_t dataId: 7;
	} data;
	uint8_t byte;
} fDataID;

typedef struct {
	uint8_t rqVersion;
	uint8_t rqTransactionId;
	uint8_t rqUnitId;
	union {
		fDataID rqFunctionId;
		uint8_t byte;
	} rqFunctionId;
	uint8_t rqData[];
} sRequest;

#define REQUEST_HEADER_SIZE ((uint8_t) offsetof(sRequest, rqData))
	
typedef struct {
	uint8_t rsVersion;
	uint8_t rsTransactionId;
	uint8_t rsCode;
	uint8_t rsData[];
} sResponse;

#define RESPONSE_HEADER_SIZE ((uint8_t) offsetof(sResponse, rsData))

enum eResponseCodes {
	ercOk = 0,
	
	ercAddresBadLength = 1,
	
	ercChBadChannels = 0x10,
	ercChBadPermissions = 0x12,
	ercChValidationFailed = 0x13,
	
	ercNotImplemented = 0x7F,
	ercBadVersion = 0x90,
	ercBadUnitId = 0xA0,
	ercNotConsecutiveTransactionId = 0xB0,
	ercBadFunctionId = 0xC0,
	ercResponseTooBig = 0xD0,

	ercBadRequestData = 0xE0,
	ercNodePermissionViolation = 0xE1,
	ercBadArguments = 0xE2,
	ercInternalError = 0xFF,
};

void generateResponse(const uint8_t requestLength, const uint8_t *requestData, uint8_t *responseLength, uint8_t *responseData);
void generateAdvertisement(uint8_t *packetLength, uint8_t *packetData);
void protocolInit();

#if defined(UNIT_TESTING) || defined(NOT_FIRMWARE)
    #pragma pack(pop)
#endif
#endif /* PROTOCOL_H_ */
