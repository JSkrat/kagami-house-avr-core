/*
 * RF_info.c
 *
 * Created: 20.04.2020 21:38:18
 *  Author: Mintytail
 */ 
#if BT_SLAVE == BUILD_TYPE

#include "RF info.h"
#include <stdint.h>


uint16_t total_requests = 0, ok_responses = 0, error_responses = 0, missed_packets = 0, ack_timeouts = 0, validation_errors = 0;

void init_rf_info() {
	total_requests = 0;
	ok_responses = 0;
	error_responses = 0;
	missed_packets = 0;
	ack_timeouts = 0;
	validation_errors = 0;
}

#endif
