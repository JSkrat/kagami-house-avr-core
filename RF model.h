/*
 * RF_parser.h
 *
 * Created: 03.11.2019 19:18:07
 *  Author: Mintytail
 *
 * The main job of this module is to create interface request-response from both, master and slave points of view
 * from the nrf24l01 library
 */ 


#ifndef RF_PARSER_H_
#define RF_PARSER_H_

#ifdef UNIT_TESTING
    #pragma pack(push, 1)
#endif

#include "../KagamiCore/defines.h"
#include "sstring.h"
#ifndef UNIT_TESTING
    #include "nRF model.h"
#else
    #include "../usb2nrf_tests/nRF model.h"
#endif


typedef enum {
	rmIdle = 0,
	rmSlave = 1,
	rmMaster = 2,
	rmGetMode // should be the last, we check for validity by mode being numerically less than that value
} eRFMode;

typedef enum {
	emNone = 0,
	emSearchMaster = 1,
	emNormalSlave = 2,
	// for validation
	emAmount
} eModeType;

//extern const eRFMode RFMode;
//extern t_address MasterAddress;

typedef uint8_t t_address[MAC_SIZE];

typedef enum {
	eptData = 0,
	eptAckOk = 1,
	eptAckTimeout = 2,
	eptResponseTimeout = 3,
} ePacketType;

// 39 bytes long, no pointers
typedef struct {
	ePacketType type; // 1
	t_address address; // 5
	uint8_t payloadLength; // 1
	uint8_t payloadData[PAYLOAD_SIZE]; // 32
} tRfPacket;

void rf_init();
void transmitPacket(tRfPacket *packet);
void nRF_listen(const uint8_t *address);
inline void RFListen(const uint8_t *address) { nRF_listen(address); }
void RFTransmit(tRfPacket *packet);
bool RFSetChannel(uint8_t channel);

void switchMode(eModeType newMode);
void setListenAddress(t_address *address);
tRfPacket* nextRFBufferElement();
eModeType getRFMode();

// only for use within RF model master/slave
extern uint8_t RFChannel;

#ifdef UNIT_TESTING
	// one call for every millisecond of time
	// for testing response timeouts in a master mode
	void msEvent(void);
	
	#pragma pack(pop)
#endif

#endif /* RF_PARSER_H_ */
