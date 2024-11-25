#ifndef IPV4_H
#define IPV4_H

#include "ethernet.h"
#include "packet.h"

#define IPV4_PROTOCOL_UDP 17

void ipv4_req(NET_PACKET* pkt, uint8_t protocol, uint32_t ip);
uint32_t ipv4_decode(void* buf);

#endif /* IPV4_H */