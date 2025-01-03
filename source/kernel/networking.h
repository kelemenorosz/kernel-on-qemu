#ifndef NETWORKING_H
#define NETWORKING_H

#include "pci.h"
#include "err.h"
#include "packet.h"
#include "list.h"

#define NETWORK_MESSAGE_PROTOCOL_UDP 0
#define NETWORK_MESSAGE_PROTOCOL_TCP 1
#define NETWORK_MESSAGE_PROTOCOL_ARP 2

typedef struct __attribute__((__packed__)) NETWORK_INTERFACE {

	void* device;
	uint8_t ether_addr[8];
	uint32_t ip_addr;
	uint32_t ip_addr_dhcp;
	uint32_t ip_addr_dns;	
	uint32_t subnet_mask;
	void(*tx_send)(void* device, NETWORK_PACKET* pkt);

} NETWORK_INTERFACE;

typedef struct __attribute__((__packed__)) NETWORK_MESSAGE {

	void* buf;
	size_t len;

} NETWORK_MESSAGE;

typedef struct __attribute__((__packed__)) NETWORK_MESSAGE_DESC {

	uint32_t protocol;
	uint32_t src_port;
	uint32_t dest_port;

} NETWORK_MESSAGE_DESC;

extern LIST* g_network_interfaces;

void networking_init();
ERR_CODE networking_init_device(PCI_ENUM_TOKEN* pci_token);
void networking_enum_interfaces();
NETWORK_INTERFACE* get_network_interface(uint32_t index);

void networking_tx_poll();
void networking_rx_sort(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* desc);

#endif /* NETWORKING_H */