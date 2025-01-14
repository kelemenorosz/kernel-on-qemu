#ifndef TCP_H
#define TCP_H

#include "packet.h"
#include "networking.h"

#define TCP_FIN 0x1
#define TCP_SYN 0x2
#define TCP_PSH 0x8
#define TCP_ACK 0x10

void tcp_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint32_t send_port, uint32_t recv_port, uint32_t ip, uint8_t flags, uint32_t seq, uint32_t ack);
ERR_CODE tcp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf, uint16_t size);



#endif /* TCP_H */