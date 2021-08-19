/*
 * RF_parser.c
 *
 * Created: 03.11.2019 19:17:51
 *  Author: Mintytail
 */
#include "../KagamiCore/RF model.h"
#include "../KagamiCore/nRF model.h"
#ifndef UNIT_TESTING
    #include <avr/interrupt.h>
	#include "Settings.h"
#else
    #include "../usb2nrf_tests/pgmspace.h"
#endif
#include <stdint.h>
#include <string.h>
#include "sstring.h"

void dataTransmitted(sString *address, sString *payload);
void dataReceived(sString *address, sString *payload);
void transmissionFailed(sString *address, sString *payload);

void rf_master_init();
void rf_slave_init();

static t_address ListenAddress;
uint8_t RFChannel;

void rf_init() {
	// setup nrf interface
	nRF_init();
	cNRF_DataReceived = &dataReceived;
	cNRF_DataTransmitted = &dataTransmitted;
	cNRF_TransmissionFailed = &transmissionFailed;
	//RFMode = rmIdle;
	readSetting(esChannel, &RFChannel);
#if BT_MASTER == BUILD_TYPE
	rf_master_init();
#elif BT_SLAVE == BUILD_TYPE
	rf_slave_init();
#endif
}

void RFTransmit(tRfPacket *packet) {
	// save the address so we could listen that address after request is sent
	memcpy(ListenAddress, packet->address, MAC_SIZE);
	nRF_transmit((uint8_t*)&(packet->address), packet->payloadLength, (uint8_t*)&(packet->payloadData));
};

bool RFSetChannel(uint8_t channel) {
#if BT_MASTER == BUILD_TYPE || BT_DEBUG == BUILD_TYPE
	if (nRF_setRFChannel(channel)) {
#elif BT_SLAVE == BUILD_TYPE
	if (nRF_validate_rf_channel(channel)) {
#endif
		saveSetting(esChannel, &channel);
		RFChannel = channel;
		return true;
	} else {
		return false;	
	}
}
