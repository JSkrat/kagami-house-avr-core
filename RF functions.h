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
#include "Settings.h"

typedef enum {
	eFSetSessionKey = 0x10,
	eFSetAddress = 0x12,
	eFGetStatistics = 0x13,
    eFResetTransactionId = 0x14,
	eFNOP = 0x15,
	eFSetRFChannel = 0x16,
	eFSetMode = 0x17,
} eU0Functions;

typedef enum {
	eFGetProperties = 0x00,
	eFGetTextDescription = 0x02,
	eFSetTextDescription = 0x03,
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

typedef struct {
	eDataType input: 4;
	eDataType output: 4;
} fDataType;

typedef struct { // 4 bytes
	eU0Functions functionCode;
	union {
		uint8_t eDataInputOutput;
		fDataType fields;
	} type;
	fRFFunction function;
} tRFCodeFunctionItem;

typedef struct {
	const uint8_t length;
	const tRFCodeFunctionItem *functions;
	const eSetting description;
} tUnit;

#define fUCount 3
extern const tRFCodeFunctionItem RFStandardFunctions[fUCount];

#define fU0Count 7
extern const tRFCodeFunctionItem RFU0Functions[fU0Count];

extern unsigned int unitsCount;
extern const tUnit *RFUnits;

void functions_init(const tUnit *units, const unsigned int number);

#endif /* FUNCTIONS_H_ */
