/*
 * functions.c
 *
 * Created: 01.01.2020 14:54:34
 *  Author: Mintytail
 */ 
#if BT_SLAVE == BUILD_TYPE

#include "../KagamiCore/RF functions.h"
#include "../KagamiCore/RF protocol.h"
#include "../KagamiCore/RF info.h"
#include "../KagamiCore/version.h"
//#include "../usb2nrf/RF custom functions.h"
#ifndef UNIT_TESTING
    #include "./RF model.h"
    #include <avr/pgmspace.h>
#else
    #include "../KagamiCore/RF model.h"
    #include "../usb2nrf_tests/pgmspace.h"
#endif
#include "../KagamiCore/RF protocol internal.h"
#include <stdbool.h>
#include <stdint.h>
// for memcpy
#include <string.h>
#include "sstring.h"
#include "Settings.h"

const tUnit *RFUnits;
unsigned int unitsCount;

void functions_init(const tUnit *units, const unsigned int number) {
	RFUnits = units;
	unitsCount = number;
}

uint8_t setSessionKey(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	// TODO after integrating cipher library
    (void) unit;
	(void) function;
    (void) request;
    (void) response;
	//saveSetting(esKey, &(request->data[0]));
	return ercNotImplemented;
}

uint8_t getNumnberOfUnits(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
    (void) unit;
	(void) function;
    (void) request;
	response->length = 5;
	response->data[0] = unitsCount;
	response->data[1] = BUILD_NUMBER & 0xFF;
	response->data[2] = (BUILD_NUMBER >> 8) & 0xFF;
	response->data[3] = (BUILD_NUMBER >> 16) & 0xFF;
	response->data[4] = (BUILD_NUMBER >> 24) & 0xFF;
	return ercOk;
}

uint8_t setMACAddress(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
    (void) unit;
	(void) function;
    (void) response;
	if (5 == request->length) {
		setListenAddress((t_address*) request->data);	
		return ercOk;
	} else {
		return ercAddresBadLength;
	}
}

uint8_t resetTransactionId(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) unit;
	(void) function;
	(void) request;
	lastTransacrionId = 128;
	response->data[0] = lastTransacrionId;
	response->length = 1;
	return ercOk;
}

uint8_t rfSetRFChannel(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) unit;
	(void) function;
	(void) response;
	if (1 != request->length) return ercBadRequestData;
	if (RFSetChannel(request->data[0]))	return ercOk;
	else return 0x80;
}

uint8_t setMode(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) unit;
	(void) function;
	(void) response;
	if (1 != request->length) return ercBadRequestData;
	switchMode(request->data[0]);
	return ercOk;
}

uint8_t rfNOP(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) unit;
	(void) function;
	(void) request;
	(void) response;
	return ercOk;
}

uint8_t getStatistics(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
    (void) unit;
	(void) function;
    (void) request;
	response->length = 12;
    #define STORE_16(index, value) response->data[index] = value & 0xFF; response->data[index+1] = ((uint16_t)value) >> 8
	STORE_16(0, total_requests);
	STORE_16(2, ok_responses);
	STORE_16(4, error_responses);
    STORE_16(6, missed_packets);
	STORE_16(8, ack_timeouts);
	STORE_16(10, validation_errors);
	return ercOk;
}

uint8_t getBuildNumber(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) unit;
	(void) function;
	(void) request;
	response->length = 4;
	response->data[0] = BUILD_NUMBER & 0xFF;
	response->data[1] = (BUILD_NUMBER >> 8) & 0xFF;
	response->data[2] = (BUILD_NUMBER >> 16) & 0xFF;
	response->data[3] = (BUILD_NUMBER >> 24) & 0xFF;
	return ercOk;
}

uint8_t getPropertiesOfUnit(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) function;
	if (0 == unit) return getNumnberOfUnits(unit, 0, request, response);
	uint8_t count;
	const tRFCodeFunctionItem *list;
	if (0 == unit) {
		count = fUCount + fU0Count;
		list = RFU0Functions;
	} else {
		count = fUCount + pgm_read_byte(&(RFUnits[unit-1].length));
		list = pgm_read_ptr(&RFUnits[unit-1].functions);
	}
	response->length = count*2;
	for (int i = 0; i < count; i++) {
		const tRFCodeFunctionItem *item;
		if (fUCount > i) item = &RFStandardFunctions[i];
		else item = &list[i-fUCount];
		fDataID d = {.byte = pgm_read_byte(&(item->dataId))};
		response->data[i*2] = ((d.data.type << fDataID_type_offset) & fDataID_type_mask)
							| ((d.data.dataId << fDataID_dataId_offset) & fDataID_dataId_mask);
		tRFCodeFunctionType t = {.byte = pgm_read_byte(&(item->type.byte))};
		switch (d.data.type) {
		case ediMethod: {
			response->data[i*2+1] = (t.methodType.input << fMethodType_input_offset) | (t.methodType.output << fMethodType_output_offset);
			break;
		}
		case ediNode: {
			response->data[i*2+1] = (t.nodeType.readable << fNodeType_readable_offset)
								  | (t.nodeType.writable << fNodeType_writable_offset)
								  | (t.nodeType.dataType << fNodeType_dataType_offset);
			break;
		}
		}
	}
	return ercOk;
}

eSetting _getUnitDescriptionId(const uint8_t unit) {
	if (0 == unit) return esU0Description;
	else return pgm_read_byte(&(RFUnits[unit-1].description));
}

uint8_t textDescription(const uint8_t unit, const uint8_t function, const scString *request, sString *response) {
	(void) function;
	if (0 == request->length) {
		response->length = readSetting(_getUnitDescriptionId(unit), &(response->data[0]));
		return ercOk;
	}
	response->length = readSetting(_getUnitDescriptionId(unit), &(response->data[0]));
	if (response->length != request->length) return ercBadRequestData;
	saveSetting(_getUnitDescriptionId(unit), &(request->data[0]));
	return ercOk;
}

const PROGMEM tRFCodeFunctionItem RFStandardFunctions[fUCount] = {
	{
		.dataId.data = {
			.type = ediNode,
			.dataId = eFProperties,
		},
		.function = &getPropertiesOfUnit, 
		.type.nodeType.dataType = edtUnspecified,
		.type.nodeType.readable = true,
		.type.nodeType.writable = false,
	},
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFTextDescription, 
		.function = &textDescription, 
		.type.nodeType.dataType = edtString,
		.type.nodeType.readable = true,
		.type.nodeType.writable = true,
	},
};

const PROGMEM tRFCodeFunctionItem RFU0Functions[fU0Count] = {
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFSessionKey, 
		.function = &setSessionKey, 
		.type.nodeType.dataType = edtByteArray,
		.type.nodeType.readable = false,
		.type.nodeType.writable = true,
	},
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFAddress, 
		.function = &setMACAddress, 
		.type.nodeType.dataType = edtByteArray,
		.type.nodeType.readable = false,
		.type.nodeType.writable = true,
	},
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFStatistics,
		.function = &getStatistics,
		.type.nodeType.dataType = edtUnspecified,
		.type.nodeType.readable = true,
		.type.nodeType.writable = false,
	},
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFRFChannel,
		.function = &rfSetRFChannel,
		.type.nodeType.dataType = edtByte,
		.type.nodeType.readable = false,
		.type.nodeType.writable = true,
	},
	{
		.dataId.data.type = ediNode,
		.dataId.data.dataId = eFMode,
		.function = &setMode,
		.type.nodeType.dataType = edtByte,
		.type.nodeType.readable = false,
		.type.nodeType.writable = true,
	},
    {
		.dataId.data.type = ediMethod,
		.dataId.data.dataId = eFResetTransactionId,
		.function = &resetTransactionId, 
		.type.methodType.input = edtNone, 
		.type.methodType.output = edtByte 
	},
    {
		.dataId.data.type = ediMethod,
		.dataId.data.dataId = eFNOP,
		.function = &rfNOP, 
		.type.methodType.input = edtNone, 
		.type.methodType.output = edtNone 
	},
};

#endif
