#ifndef IPV4_H
#define IPV4_H

#include "ethernet.h"
#include "packet.h"
#include "socket.h"
#include "err.h"

#define IPV4_PROTOCOL_TCP 6
#define IPV4_PROTOCOL_UDP 17

void ipv4_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint8_t protocol, uint32_t ip);
ERR_CODE ipv4_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf);

#endif /* IPV4_H */