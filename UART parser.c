/*
 * UART_parser.c
 *
 * Created: 13.01.2019 16:25:04
 *  Author: J-sama
 */ 
#if BT_MASTER == BUILD_TYPE || BT_DEBUG == BUILD_TYPE

#include "UART parser.h"
#include "UART protocol.h"
#include "defines.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>

#define U_TRANSMIT_START UCSR0B |= (1 << UDRIE0)

void uSendPacket(uPackage *packet);
void uQueueChar(const uint8_t c);
void parse(unsigned char b);

static bool escActive;
uPackage respBuffer;

// +1 for make sure begin and end will be equal only at empty buffer
#define SEND_BUFFER_SIZE ((1 + MAC_SIZE + PAYLOAD_SIZE + 1)*2)
uint8_t sendBuffer[SEND_BUFFER_SIZE];
/// sendBufferBegin points to next character to send
/// sendBufferEnd points to next empty space in buffer
uint8_t sendBufferBegin = 0, sendBufferEnd = 0;

uint8_t replyAddress[MAC_SIZE];
uint16_t badRFPackets = 0;
uint16_t rfTimeouts = 0;
uint16_t rfPacketsSent = 0;

void u_init() {
	DDRD = (1 << PORTD1);
	// USART
	#define MYUBRR (F_CPU/8/BAUDRATE-1)
	UBRR0H = (unsigned char) (MYUBRR>>8);
	UBRR0L = (unsigned char) (MYUBRR);
	// double speed, usart enable
	UCSR0A = (1 << U2X0);
	// enable rx and interrupts
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (0 << TXCIE0) | (0 << UDRIE0);
	// asynchronous, 8N1
	UCSR0C = (0b00 << UMSEL00) | (0b11 << UCSZ00) | (0b00 << UPM00) | (0 << USBS0);
	
	// send invalid response to ensure uart settings are ok
	uQueueChar(0xC0);
	uQueueChar(0x00);
	uQueueChar(0x00);
	U_TRANSMIT_START;
}

void uQueueChar(const uint8_t c) {
	/// TODO check "buffer full" case and show on screen and turn on red for some time
	// first put character
	sendBuffer[sendBufferEnd] = c;
	// then move pointer so it won't send garbage
	sendBufferEnd++;
	if (SEND_BUFFER_SIZE <= sendBufferEnd) sendBufferEnd = 0;	
}

void uQueueBytes(const uint8_t *bytes, const uint8_t number) {
	for (uint8_t p = 0; p < number; p++) {
		uQueueChar(bytes[p]);
	}
}

ISR(USART_RX_vect) {
	// on not empty receive buffer
	// error flags for current UDR0, must be read before reading UDR0
	// UCSR0A: FE0 frame error (stop bit is not 1), DOR0 data overrun, UPE0 parity error
	parse(UDR0);
}

ISR(USART_UDRE_vect) {
	/// when data register is empty and ready to send another byte
	// check if buffer is empty
	if (sendBufferBegin == sendBufferEnd) {
		UCSR0B &= ~(1 << UDRIE0);
		return;
	}
	UDR0 = sendBuffer[sendBufferBegin++];
	if (SEND_BUFFER_SIZE <= sendBufferBegin) sendBufferBegin = 0;
}

void uSendPacket(uPackage *packet) {
	uQueueChar(0xC0);
	for (int8_t i = 0; i < packet->pkg.payloadSize + RESPONSE_HEADER_SIZE; i++) {
		char c = packet->packageBuffer[i];
		if (0xC0 == c) {
			uQueueChar(0xDB);
			c = 0xDC;
		} else if (0xDB == c) {
			uQueueChar(0xDB);
			c = 0xDD;
		}
		uQueueChar(c);
	}
	U_TRANSMIT_START;
}

/** \brief Parse commands from USART
 */
void parse(unsigned char b) {
	// byte stuffing (or escaping) first
	switch (b) {
		case 0xC0: {
			UARTBeginTransaction();
			escActive = false;
			return;
		}
		case 0xDB: {
			escActive = true;
			return;
		}
		case 0xDC: {
			if (escActive) {
				b = 0xC0;
				escActive = false;
			}
			break;
		}
		case 0xDD: {
			if (escActive) {
				b = 0xDB;
				escActive = false;
			}
			break;
		}
	}
	if (UARTProcessNextByte(b, &respBuffer)) {
		uSendPacket(&respBuffer);
	}	
}

#endif