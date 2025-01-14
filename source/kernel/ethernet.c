#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ethernet.h"
#include "memory.h"
#include "netswap.h"
#include "serial.h"
#include "interrupt.h"
#include "ipv4.h"
#include "arp.h"

typedef struct __attribute__((__packed__)) ETHER_HEADER {

	uint8_t dest_addr[6];
	uint8_t src_addr[6];
	uint16_t ether_type;

} ETHER_HEADER;

void ether_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint16_t ether_type, uint32_t ip) {

	pkt->start -= sizeof(ETHER_HEADER);
	ETHER_HEADER* ether_header = (ETHER_HEADER*)pkt->start;

	if (ether_type == ETHER_TYPE_IPV4) {

		if (ip == 0) {
			memset((void*)&ether_header->dest_addr[0], 0xFF, 6); 
		}
		else {
			arp_lookup(ip, (void*)&ether_header->dest_addr[0]);
		}
		memcpy((void*)&ether_header->src_addr[0], (void*)&intf->ether_addr[0], 6);
		ether_header->ether_type = netswap16(ether_type);

	}
	else if (ether_type == ETHER_TYPE_ARP) {

		memset((void*)&ether_header->dest_addr[0], 0xFF, 6);
		memcpy((void*)&ether_header->src_addr[0], (void*)&intf->ether_addr[0], 6);
		ether_header->ether_type = netswap16(ether_type);

	}

	return;
}

ERR_CODE ether_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf) {

	ETHER_HEADER* ether_header = (ETHER_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with destination mac address ");
	for (uint8_t i = 0; i < 6; ++i) {
		serial_write_byte(ether_header->dest_addr[i]);
		if (i != 5) serial_write_string(":");
	}
	serial_write_newline();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with source mac address ");
	for (uint8_t i = 0; i < 6; ++i) {
		serial_write_byte(ether_header->src_addr[i]);
		if (i != 5) serial_write_string(":");
	}
	serial_write_newline();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with ether type 0x");
	serial_write_word(netswap16(ether_header->ether_type));
	serial_write_newline();
	enable_interrupts();

	if (netswap16(ether_header->ether_type) == ETHER_TYPE_IPV4) {

		return ipv4_decode(msg, msg_desc, buf + sizeof(ETHER_HEADER));

	}
	else if (netswap16(ether_header->ether_type) == ETHER_TYPE_ARP) {

		return arp_decode(msg, msg_desc, buf + sizeof(ETHER_HEADER));
	}

	return E_FAIL;

}
