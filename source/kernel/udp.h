#ifndef UDP_H
#define UDP_H

#include "ethernet.h"
#include "packet.h"

void udp_req(NET_PACKET* pkt, uint32_t send_port, uint32_t recv_port, uint32_t ip);
uint32_t udp_decode(void* buf);

#endif /* UDP_H */