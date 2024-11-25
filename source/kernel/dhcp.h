#ifndef DHCP_H
#define DHCP_H

#include "ethernet.h"
#include "packet.h"

uint8_t dhcp_client();
uint32_t dhcp_req(void* buf, uint32_t req);

#endif /* DHCP_H */