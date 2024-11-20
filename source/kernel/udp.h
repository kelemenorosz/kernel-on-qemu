#ifndef UDP_H
#define UDP_H

#include "ethernet.h"
#include "packet.h"

void req_udp(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet, uint16_t src_port, uint16_t dest_port);

#endif /* UDP_H */