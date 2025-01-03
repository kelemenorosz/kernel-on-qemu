#ifndef UDP_H
#define UDP_H

#include "ethernet.h"
#include "packet.h"
#include "networking.h"
#include "err.h"

void udp_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint32_t send_port, uint32_t recv_port, uint32_t ip);
ERR_CODE udp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf);

#endif /* UDP_H */