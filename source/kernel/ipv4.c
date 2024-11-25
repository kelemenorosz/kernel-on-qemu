#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ethernet.h"
#include "packet.h"
#include "netswap.h"
#include "checksum.h"
#include "serial.h"
#include "interrupt.h"
#include "ipv4.h"
#include "udp.h"

typedef struct __attribute__((__packed__)) IPV4_HEADER {

	uint8_t version_ihl;
	uint8_t tos;
	uint16_t tol;
	uint16_t identification;
	uint16_t fragment_offset;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint32_t src_addr;
	uint32_t dest_addr;

} IPV4_HEADER;

void ipv4_req(NET_PACKET* pkt, uint8_t protocol, uint32_t ip) {

	pkt->start -= sizeof(IPV4_HEADER);
	IPV4_HEADER* ipv4_header = (IPV4_HEADER*)pkt->start;

	ipv4_header->version_ihl = (4 << 4) | 5; // Version 4; Header length 5
	ipv4_header->tos = 0x10;
	ipv4_header->tol = netswap16(pkt->end - pkt->start);
	ipv4_header->identification = 0;
	ipv4_header->fragment_offset = 0;
	ipv4_header->ttl = 64;
	ipv4_header->protocol = protocol;
	ipv4_header->checksum = 0;
	ipv4_header->src_addr = 0; // Add client ipv4 addr later
	if (ip == 0) {
		ipv4_header->dest_addr = 0xFFFFFFFF; // Broadcast
	}

	uint16_t checksum = netchecksum(pkt->start, pkt->start + sizeof(IPV4_HEADER));
	ipv4_header->checksum = netswap16(checksum);

	ether_req(pkt, ETHER_TYPE_IPV4);

	return;

}

uint32_t ipv4_decode(void* buf) {

	IPV4_HEADER* ipv4_header = (IPV4_HEADER*)buf;
	uint8_t* addr = NULL;

	disable_interrupts();
	serial_write_string("[RX_IPV4_HEADER] IPv4 header with destination IP address ");
	addr = (uint8_t*)&ipv4_header->dest_addr;
	for (uint8_t i = 0; i < 4; ++i) {
		serial_write_byte(addr[i]);
		if (i != 3) serial_write_string(".");
	}
	serial_write_newline();
	serial_write_string("[RX_IPV4_HEADER] IPv4 header with source IP address ");
	addr = (uint8_t*)&ipv4_header->src_addr;
	for (uint8_t i = 0; i < 4; ++i) {
		serial_write_byte(addr[i]);
		if (i != 3) serial_write_string(".");
	}
	serial_write_newline();
	enable_interrupts();

	if (ipv4_header->protocol == IPV4_PROTOCOL_UDP) {

		return udp_decode(buf + sizeof(IPV4_HEADER));

	}

	return 0;

}