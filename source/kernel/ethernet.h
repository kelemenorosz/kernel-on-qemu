#ifndef ETHERNET_H
#define ETHERNET_H

#include "pci.h"
#include "packet.h"

#define ETHERNET_TRANSPORT_PROTOCOL_UDP 0
#define ETHERNET_TRANSPORT_PROTOCOL_TCP 1

typedef struct __attribute__((__packed__)) RX_DESCRIPTOR {

	volatile uint32_t addr_low;
	volatile uint32_t addr_high;
	volatile uint16_t length;
	volatile uint16_t checksum;
	volatile uint8_t status;
	volatile uint8_t errors; 
	volatile uint16_t special;

} RX_DESCRIPTOR;

typedef struct __attribute__((__packed__)) TX_DESCRIPTOR {

	volatile uint32_t addr_low;
	volatile uint32_t addr_high;
	volatile uint16_t length;
	volatile uint8_t cso;
	volatile uint8_t command;
	volatile uint8_t status; 
	volatile uint8_t css;
	volatile uint16_t special;

} TX_DESCRIPTOR;

typedef struct __attribute__((__packed__)) ETHERNET_DEVICE {

	RX_DESCRIPTOR* rxdl;
	TX_DESCRIPTOR* txdl;
	uint32_t tx_tail;
	uint8_t mac_addr[6];

} ETHERNET_DEVICE;

typedef struct __attribute__((__packed__)) SOCKET {

	uint32_t ip;
	uint32_t port;
	uint32_t protocol;
	void* recv_buffer;
	struct SOCKET* sck_next;

} SOCKET;

typedef struct __attribute__((__packed__)) SOCKET_LIST {

	SOCKET* sck_head;

} SOCKET_LIST;

#define ETHER_TYPE_IPV4 0x0800

void ethernet_init(PCI_ENUM_TOKEN* token);
SOCKET* ksocket(uint32_t ip, uint32_t port, uint32_t protocol); 
void ksend(SOCKET* sck, void* buf, uint32_t buf_len, uint32_t send_port, uint32_t ip);
void* kreceive(SOCKET* sck);

void ether_req(NET_PACKET* pkt, uint16_t ether_type);
uint32_t ether_decode(void* buf);

#endif /* ETHERNET_H */