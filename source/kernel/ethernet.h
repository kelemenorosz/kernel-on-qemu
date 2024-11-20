#ifndef ETHERNET_H
#define ETHERNET_H

#include "pci.h"

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

void ethernet_init(PCI_ENUM_TOKEN* token);

#endif /* ETHERNET_H */