#ifndef DHCP_H
#define DHCP_H

#include "ethernet.h"
#include "packet.h"

void req_dhcp(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet);

#endif /* DHCP_H */