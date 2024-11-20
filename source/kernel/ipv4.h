#ifndef IPV4_H
#define IPV4_H

#include "ethernet.h"
#include "packet.h"

#define IPV4_PROTOCOL_UDP 17

void req_ipv4(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet, uint8_t protocol);

#endif /* IPV4_H */