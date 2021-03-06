/*
 * functions.h
 *
 * Created: 01.01.2020 14:54:46
 *  Author: Mintytail
 */ 


#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "../KagamiCore/RF protocol.h"
#include <stdint.h>
#include <stdbool.h>
#include "Settings.h"
#if defined(UNIT_TESTING) || defined(NOT_FIRMWARE)
	#pragma pack(push,1)
#endif

// nodes are 0 - 7F, methods are 80 - FF
typedef enum {
	// nodes
	eFSessionKey = 0x10,
	eFAddress = 0x12,
	eFStatistics = 0x13,
	eFRFChannel = 0x16,
	eFMode = 0x17,
	// methods, separate namespace
	eFNOP = 0x10,
    eFResetTransactionId = 0x11,
} eU0Functions;

typedef enum {
	eFProperties = 0x00,
	eFTextDescription = 0x02,
} eStandartFunctions;

// up to 16 items
typedef enum {
	edtNone = 0,
	edtBool = 1,
	edtByte = 2,
	edtInt32 = 3,
	edtString = 4,
	edtByteArray = 5,
	edtUnspecified = 0xF
} eDataType;

#define fMethodType_input_offset 4
#define fMethodType_output_offset 0
typedef struct {
	eDataType input: 4;
	eDataType output: 4;
} fMethodType;

#define fNodeType_readable_offset 7
#define fNodeType_writable_offset 6
#define fNodeType_dataType_offset 0
typedef struct {
	bool readable: 1;
	bool writable: 1;
	eDataType dataType: 4;
} fNodeType;

typedef union {
	uint8_t byte;
	fMethodType methodType;
	fNodeType nodeType;
} tRFCodeFunctionType;

typedef struct { // 4 bytes
	fDataID dataId;
	tRFCodeFunctionType type;
	fRFFunction function;
} tRFCodeFunctionItem;

typedef struct {
	const uint8_t length;
	const tRFCodeFunctionItem *functions;
	const eSetting description;
} tUnit;

#define fUCount 2
extern const tRFCodeFunctionItem RFStandardFunctions[fUCount];

#define fU0Count 7
extern const tRFCodeFunctionItem RFU0Functions[fU0Count];

extern unsigned int unitsCount;
extern const tUnit *RFUnits;

void functions_init(const tUnit *units, const unsigned int number);

#if defined(UNIT_TESTING) || defined(NOT_FIRMWARE)
	#pragma pack(pop)
#endif
#endif /* FUNCTIONS_H_ */
