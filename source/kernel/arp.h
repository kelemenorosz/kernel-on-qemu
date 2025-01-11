#ifndef ARP_H
#define ARP_H

#include "err.h"
#include "networking.h"

ERR_CODE arp_lookup(uint32_t ip, uint8_t* eth_addr);
uint32_t arp_req(NETWORK_INTERFACE* intf, void* buf, uint32_t ip);

ERR_CODE arp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf);
void arp_sort(NETWORK_MESSAGE* msg, void* device, void(*rar_add)(void* device, void* eth_addr));

#endif /* ARP_H */