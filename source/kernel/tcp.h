#ifndef TCP_H
#define TCP_H

#include "packet.h"

#define TCP_FIN 0x1
#define TCP_SYN 0x2
#define TCP_PSH 0x8
#define TCP_ACK 0x10

typedef struct __attribute__((__packed__)) TCP_HEADER {

	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq_number;
	uint32_t ack_number;
	uint8_t data_offset;
	uint8_t flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_pointer;

} TCP_HEADER;

// void tcp_req(NET_PACKET* pkt, uint32_t send_port, uint32_t recv_port, uint32_t ip, uint8_t flags, uint32_t seq, uint32_t ack);
// uint8_t tcp_decode(void* buf, size_t size);

#endif /* TCP_H */