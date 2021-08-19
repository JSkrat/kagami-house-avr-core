/*
 * UART_functions.c
 *
 * Created: 13.04.2020 22:07:48
 *  Author: Mintytail
 */ 
#if BT_MASTER == BUILD_TYPE || BT_DEBUG == BUILD_TYPE

#include "UART functions.h"
#include "sstring.h"
#include "UART protocol.h"
#include "../KagamiCore/nrf24l01-mnemonics.h"
#include "../KagamiCore/nrf24l01.h"
#include "../KagamiCore/nRF model.h"
#include "../KagamiCore/RF model.h"
#include "../KagamiCore/version.h"
#include <string.h>
#ifndef UNIT_TESTING
    #include <avr/pgmspace.h>
#else
    #include "../usb2nrf_tests/pgmspace.h"
#endif

uint8_t uStatus(const scString *request, sString *response) {
	response->length = 0x13;
	for (int i = 0x00; i <= 0x09; i++) {
		nRF24L01_read_register(rfTransiever, i, &(response->data[i]), 1);
	}
	for (int i = 0x11; i <= 0x17; i++) {
		nRF24L01_read_register(rfTransiever, i, &(response->data[i - 0x11 + 0x0A]), 1);
	}
	for (int i = 0x1C; i <= 0x1D; i++) {
		nRF24L01_read_register(rfTransiever, i, &(response->data[i - 0x1C + 0x11]), 1);
	}
	//*((uint16_t*) (respBuffer.pkg.payload+1)) = rfPacketsSent;
	//*((uint16_t*) (respBuffer.pkg.payload+3)) = rfTimeouts;
	//*((uint16_t*) (respBuffer.pkg.payload+5)) = badRFPackets;
	return eucOk;
}

uint8_t uVersion(const scString *request, sString *response) {
	response->length = 4;
	*((uint32_t*) response->data) = BUILD_NUMBER;
	return eucOk;
}

uint8_t uAddresses(const scString *request, sString *response) {
	response->length = 0x13;
	nRF24L01_read_register(rfTransiever, 0x0A, &(response->data[0x00]), 5);
	nRF24L01_read_register(rfTransiever, 0x0B, &(response->data[0x05]), 5);
	for (int i = 0x0C; i < 0x10; i++) {
		nRF24L01_read_register(rfTransiever, i, &(response->data[i - 0x0C + 0x0A]), 1);
	}
	nRF24L01_read_register(rfTransiever, 0x10, &(response->data[0x0E]), 5);
	return eucOk;
}

uint8_t uSetChannel(const scString *request, sString *response) {
	nRF24L01_write_register(rfTransiever, RF_CH, (uint8_t*) &(request->data[0]), 1);
	return eucOk;
}

uint8_t uSetBitRate(const scString *request, sString *response) {
	uint8_t rf_setup; 
	nRF24L01_read_register(rfTransiever, RF_SETUP, &rf_setup, 1);
	rf_setup &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
	if (request->data[0] & 0b00000010) rf_setup |= (1 << RF_DR_LOW);
	if (request->data[0] & 0b00000001) rf_setup |= (1 << RF_DR_HIGH);
	nRF24L01_write_register(rfTransiever, RF_SETUP, &rf_setup, 1);
	return eucOk;
}

uint8_t uSetTxPower(const scString *request, sString *response) {
	uint8_t rf_setup;
	nRF24L01_read_register(rfTransiever, RF_SETUP, &rf_setup, 1);
	rf_setup &= ~(0b00000011 << RF_PWR);
	rf_setup |= (request->data[0] & 0b00000011) << RF_PWR;
	nRF24L01_write_register(rfTransiever, RF_SETUP, &rf_setup, 1);
	return eucOk;
}

uint8_t uClearTx(const scString *request, sString *response) {
	nRF24L01_flush_transmit_message(rfTransiever);
	return eucOk;
}

uint8_t uListen(const scString *request, sString *response) {
	RFListen(request->data);
	return eucOk;
}

uint8_t uSetMode(const scString *request, sString *response) {
	return eucNotImplemented;
	//response->length = 1;
	//eRFMode setMode = request->data[0];
	//eRFMode newMode = switchRFMode(setMode);
	//response->data[0] = newMode;
	//if (setMode == newMode) {
	//	return eucOk;
	//} else {
	//	return eucGeneralFail;
	//}
}

uint8_t uSetListenAddress(const scString *request, sString *response) {
	return eucNotImplemented;
	//setListenAddress((t_address *) request->data);
	//return eucOk;
}

uint8_t readRFBuffer(const scString *request, sString *response) {
	// we're in the interrupt handler, so no interrupts until we finish
	// so, pointer to the packet is valid until then :)
	tRfPacket *data = nextRFBufferElement();
	if (NULL == data) {
		return eucNoPackets;
	} else {
		response->length = data->payloadLength + MAC_SIZE;
		memcpy(&response->data[0], data->address, MAC_SIZE);
		if (data->payloadLength)
			memcpy(&response->data[MAC_SIZE], data->payloadData, data->payloadLength);
		switch (data->type) {
			case eptData: return eucDataPacket;
			case eptResponseTimeout: return eucSlaveResponseTimeout;
			case eptAckOk: return eucAckPacket;
			case eptAckTimeout: return eucAckTimeout;
			default: return eucGeneralFail;
		}
	}
}

uint8_t uTransmit(const scString *request, sString *response) {
	//response->length = 1;
	if (MAC_SIZE > request->length) {
		return eucArgumentValidationError;
	}
	tRfPacket packet;
	memcpy(packet.address, request->data, MAC_SIZE);
	packet.payloadLength = request->length - MAC_SIZE;
	if (packet.payloadLength) {
		memcpy(packet.payloadData, request->data + MAC_SIZE, packet.payloadLength);
	}
	RFTransmit(&packet);
	return eucOk;
}

const PROGMEM tUARTCommandItem UARTFunctions[UART_FUNCTIONS_NUMBER] = {
	{ mcStatus, &uStatus },
	{ mcVersion, &uVersion },
	{ mcAddresses, &uAddresses },
		
	{ mcSetChannel, &uSetChannel },
	{ mcSetTXPower, &uSetTxPower },
	{ mcSetBitRate, &uSetBitRate },
	{ mcSetAutoRetransmitDelay, NULL },
	{ mcSetAutoRetransmitCount, NULL },
		
	{ mcClearTX, &uClearTx },
	{ mcClearRX, NULL },
		
	{ mcListen, &uListen },
		
	{ mcSetMode, &uSetMode },
	{ mcSetListenAddress, &uSetListenAddress },
	
    { mcReadRFBuffer, &readRFBuffer },
	
	{ mcTransmit, &uTransmit },

    { mcEcho, NULL },
};

#endif