/*
 * RF_model_master.c
 *
 * Created: 16.05.2020 16:37:43
 *  Author: Mintytail
 */ 
#include "defines.h"
#if BT_MASTER == BUILD_TYPE || defined UNIT_TESTING
#include "../KagamiCore/RF model.h"
#ifndef UNIT_TESTING
	#include <avr/interrupt.h>
#else
	#include "../usb2nrf_tests/pgmspace.h"
#endif
#include <stdint.h>
#include <string.h>
#include "sstring.h"

static inline void msEvent(void);

// received packages or events buffer
// outside module shall get data from here by nextRFBufferElement
#define RFBUFFER_SIZE 8
static tRfPacket RFBuffer[RFBUFFER_SIZE];
// push to the end, pop from the begin
// begin points to the next item
// end points to the next free space
static int rfbBegin = 0, rfbEnd = 0, rfbSize = 0;
static int responseTimeout; // negative value means it is disabled, event triggered when it becomes disabled

#ifndef UNIT_TESTING
#if 0 == RF_TIMER
ISR(TIMER0_COMPA_vect) {
#elif 2 == RF_TIMER
ISR(TIMER2_COMPA_vect) {
#endif
	// this is being called every millisecond
	msEvent();
}
#endif

void rf_master_init() {
	// reset buffer
	rfbBegin = 0; rfbEnd = 0; rfbSize = 0;
	responseTimeout = -1;
#ifndef UNIT_TESTING
	// now let's setup timer
#if 0 == RF_TIMER
	TCCR0A = (1 << WGM01) | (0 << WGM00); // CTC mode
	TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00) | (0 << WGM02); // clk_io/64
	OCR0A = 250; // compare match interrupt every millisecond
	TIMSK0 = (1 << OCIE0A);
#elif 2 == RF_TIMER
	TCCR2A = (1 << WGM21) | (0 << WGM20); // CTC mode
	TCCR2B = (0 << CS22) | (1 << CS21) | (1 << CS20) | (0 << WGM22); // clk_io/64
	OCR2A = 250; // compare match interrupt every millisecond
	TIMSK2 = (1 << OCIE2A);
#endif
#endif
}

/**
 * RF buffer functions
 */
tRfPacket* nextRFBufferFreeElement() {
	if (RFBUFFER_SIZE <= rfbSize) return NULL;
	rfbSize += 1;
	tRfPacket *ret = &(RFBuffer[rfbEnd]);
	rfbEnd += 1;
	if (RFBUFFER_SIZE <= rfbEnd) rfbEnd = 0;
	return ret;
}

/// get next element from FIFO
/// not threadsafe, so only can be called from within the interrupt handler, under CLI
tRfPacket* nextRFBufferElement() {
	if (0 >= rfbSize) return NULL;
	rfbSize -= 1;
	tRfPacket *ret = &(RFBuffer[rfbBegin]);
	rfbBegin += 1;
	if (RFBUFFER_SIZE <= rfbBegin) rfbBegin = 0;
	return ret;
}

uint8_t RFBufferSpaceLeft() {
	return RFBUFFER_SIZE - rfbSize;
}

void dataReceived(sString *address, sString *payload) {
	// event, one per received packet
	tRfPacket *request = nextRFBufferFreeElement();
	if (NULL == request) {
		RF_ERROR(ereReceiveBufferOverflow);
		return;
	}
	request->type = eptData;
	memcpy(request->address, address->data, address->length);
	request->payloadLength = payload->length;
	memcpy(&(request->payloadData), payload->data, payload->length);
	// disable timeout, we've received response
	responseTimeout = -1;
}


void dataTransmitted(sString *address, sString *payload) {
	(void) payload; // *payload will contain ACK payload when we will support it
	// ack received, tx successful
	tRfPacket *request = nextRFBufferFreeElement();
	if (NULL == request) {
		RF_ERROR(ereTransmitBufferOverflow);
		return;
	}
	request->type = eptAckOk;
	request->payloadLength = 0;
	memcpy(&(request->address), address->data, MAC_SIZE);
	// and here we're waiting for the response, but not forever
	responseTimeout = SLAVE_RESPONSE_TIMEOUT_MS;
	RFListen((uint8_t*) &ListenAddress);
}

void transmissionFailed(sString *address, sString *payload) {
	(void) payload;
	// no ack received n times
	tRfPacket *request = nextRFBufferFreeElement();
	if (NULL == request) {
		RF_ERROR(ereTransmitFailedBufferOverflow);
		return;
	}
	request->type = eptAckTimeout;
	request->payloadLength = 0;
	memcpy(&(request->address), address->data, MAC_SIZE);
}

static inline void responseTimeoutEvent() {
	// no response from the requested slave
	tRfPacket *packet = nextRFBufferFreeElement();
	if (NULL == packet) {
		RF_ERROR(ereResponseTimeoutBufferOverflow);
		return;
	}
	packet->type = eptResponseTimeout;
	packet->payloadLength = 0;
	memcpy(&(packet->address), ListenAddress, MAC_SIZE);
}

static inline void msEvent() {
	if (0 <= responseTimeout) {
		if (0 > --responseTimeout) {
			responseTimeoutEvent();
		}
	}
}


#endif