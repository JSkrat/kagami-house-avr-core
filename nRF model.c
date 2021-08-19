/*
 * RF_statistics.c
 *
 * Created: 05.03.2020 21:54:49
 *  Author: Mintytail
 */ 

#include <stdint.h>
#include <string.h>
#ifndef UNIT_TESTING
	#include <avr/interrupt.h>
	#include <util/delay.h>
#endif
#include "../KagamiCore/nrf24l01-mnemonics.h"
#include "../KagamiCore/nrf24l01.h"
#include "../KagamiCore/defines.h"
#include "../KagamiCore/nRF model.h"
#include "../KagamiCore/RF info.h"

nRF24L01 *rfTransiever;
fNRFCallback cNRF_DataTransmitted = NULL, cNRF_DataReceived = NULL, cNRF_TransmissionFailed = NULL;
static uint8_t rf_channel;
bool receivedDataPresent();

inline static void set_low(gpio_pin pin) {
	*pin.port &= ~_BV(pin.pin);
}

void nRF_init() {
	// nrf24l01
	// interrupt from pcint0 falling edge
	PCICR = (1 << PCIE0);
	// pin b0 change interrupt enable
	uint8_t wr = 0;
	PCMSK0 = (1 << PCINT0);
	rfTransiever = nRF24L01_init();
	rfTransiever->ss.port = &portTransiever; rfTransiever->ss.pin = PORTB1;
	rfTransiever->ce.port = &portTransiever; rfTransiever->ce.pin = PORTB2;
	rfTransiever->mosi.port = &portTransiever; rfTransiever->mosi.pin = PORTB3;
	rfTransiever->mosi.port = &portTransiever; rfTransiever->miso.pin = PORTB4;
	// turn on pull-up for miso, to increase spi speed, maybe
	// internal pull up resistor is 20k to 50k
	PORTB |= _BV(rfTransiever->miso.pin);
	rfTransiever->sck.port = &portTransiever; rfTransiever->sck.pin = PORTB5;
	// init by invalid value to guarantee register update at first channel switch
	rf_channel = 255;
	nRF24L01_begin(rfTransiever);
	// disable auto ack
	wr = 0; nRF24L01_write_register(rfTransiever, EN_AA, &wr, 1);
	// 1Mbps, max power
	wr = 3 << RF_PWR; nRF24L01_write_register(rfTransiever, RF_SETUP, &wr, 1);
	// ARD 1ms, ARC 5 (no need without auto ack)
	//wr = 5 | 3 << ARD; nRF24L01_write_register(rfTransiever, SETUP_RETR, &wr, 1);
	// enable pipe 0 receive (ACK won't get to us without that)
	wr = _BV(0); nRF24L01_write_register(rfTransiever, EN_RXADDR, &wr, 1);
}

ISR(PCINT0_vect) {
	// pin change, but we need only falling  edge
	if (PORT_IN_TRANSIEVER & (1 << poTransiever_IRQ)) return;
	// The IRQ pin is activated when TX_DS IRQ, RX_DR IRQ or MAX_RT IRQ are set high
	// by the state machine in the STATUS register
	set_low(rfTransiever->ce);
	nRF24L01_update_status(rfTransiever);
	uint8_t addressBytes[MAC_SIZE];
	sString address = {MAC_SIZE, &addressBytes[0]};
	if (rfTransiever->status & _BV(TX_DS)) {
		// Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
		// this bit is set high only when ACK is received.
		nRF24L01_read_register(rfTransiever, TX_ADDR, address.data, MAC_SIZE);
		uint8_t data = _BV(TX_DS);
		nRF24L01_write_register(rfTransiever, STATUS, &data, 1);
		/// @todo there instead of NULL should be ack payload, if there is any
		if (cNRF_DataTransmitted) (*cNRF_DataTransmitted)(&address, NULL);
	}
	if (rfTransiever->status & _BV(MAX_RT)) {
		// Maximum number of TX retransmits interrupt
		// If MAX_RT is asserted it must be cleared to enable further communication.
		nRF24L01_read_register(rfTransiever, TX_ADDR, address.data, MAC_SIZE);
		// TX FIFO does not pop failed element, if we won't clean it, it will be re-sent again
		nRF24L01_flush_transmit_message(rfTransiever);
		uint8_t data = _BV(MAX_RT);
		nRF24L01_write_register(rfTransiever, STATUS, &data, 1);
		if (cNRF_TransmissionFailed) (*cNRF_TransmissionFailed)(&address, NULL);
	}
	if (rfTransiever->status & _BV(RX_DR)) {
		// Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO
		// The RX_DR IRQ is asserted by a new packet arrival event.
		// The procedure for handling this interrupt should be:
		// 1) read payload through SPI,
		// 2) clear RX_DR IRQ,
		// 3) read FIFO_STATUS to check if there are more payloads available in RX FIFO,
		// 4) if there are more data in RX FIFO, repeat from step 1).
		sString payload;
		nRF24L01Message msg;
		do {
			nRF24L01_read_received_data(rfTransiever, &msg);
			payload.length = msg.length;
			memcpy(payload.data, &(msg.data), msg.length);
			// read pipe 1 address first, so if it is 2-5 we could overwrite last byte to make correct address
			//if (0 != msg.pipe_number) nRF24L01_read_register(rfTransiever, RX_ADDR_P1, address.data, MAC_SIZE);
			switch (msg.pipe_number) {
				case 0: {
					nRF24L01_read_register(rfTransiever, RX_ADDR_P0, address.data, MAC_SIZE);
					break;
				}
				nRF24L01_read_register(rfTransiever, RX_ADDR_P1, address.data, MAC_SIZE);
				// pipe 1 address is already pre-filled, so do nothing
				case 1: break;
				// overwrite last byte of address
				case 2:
				case 3:
				case 4:
				case 5: {
					nRF24L01_read_register(rfTransiever, RX_ADDR_P2 - 2 + msg.pipe_number, &(address.data[MAC_SIZE-1]), 1);
					break;
				}
			}
			//uint8_t data = _BV(RX_DR);
			//nRF24L01_write_register(rfTransiever, STATUS, &data, 1);
			if (cNRF_DataReceived) (cNRF_DataReceived)(&address, &payload);
		} while (receivedDataPresent());
	}

	//checkTransieverRXBuf(/*true*/);
}

void nRF_listen(const uint8_t *address) {
	uint8_t config;
	nRF24L01_read_register(rfTransiever, CONFIG, &config, 1);
    config |= _BV(PRIM_RX);
    nRF24L01_write_register(rfTransiever, CONFIG, &config, 1);
	nRF24L01_listen(rfTransiever, 0, (uint8_t *)address);
}

void nRF_transmit(uint8_t *address, uint8_t length, uint8_t *data) {
	// without CE changing from low to high transmission won't start
	set_low(rfTransiever->ce);
	_delay_us(10);
	// @todo optimize the library to avoid unnecessary copying here
	nRF24L01Message msg;
	msg.length = length;
	memcpy(&(msg.data), data, length);
	nRF24L01_transmit(rfTransiever, address, &msg);
}

bool receivedDataPresent() {
	nRF24L01_update_status(rfTransiever);
	return nRF24L01_pipe_number_received(rfTransiever) >= 0;
}

void nRF_go_idle() {
	set_low(rfTransiever->ce);
}

bool nRF_validate_rf_channel(uint8_t channel) {
	return channel < 128;
}

bool nRF_setRFChannel(uint8_t channel) {
	if (nRF_validate_rf_channel(channel)) {
		if (rf_channel != channel) {
			nRF24L01_write_register(rfTransiever, RF_CH, &channel, 1);
			rf_channel = channel;
		}
		return true;
	} else {
		return false;
	}
}
