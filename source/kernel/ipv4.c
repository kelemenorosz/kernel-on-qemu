#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ethernet.h"
#include "packet.h"
#include "netswap.h"
#include "checksum.h"

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

void req_ipv4(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet, uint8_t protocol) {

	packet->start -= sizeof(IPV4_HEADER);
	IPV4_HEADER* ipv4_header = (IPV4_HEADER*)packet->start;

	ipv4_header->version_ihl = (4 << 4) | 5; // Version 4; Header length 5
	ipv4_header->tos = 0x10;
	ipv4_header->tol = netswap16(packet->end - packet->start);
	ipv4_header->identification = 0;
	ipv4_header->fragment_offset = 0;
	ipv4_header->ttl = 64;
	ipv4_header->protocol = protocol;
	ipv4_header->checksum = 0;
	ipv4_header->src_addr = 0; // Add client ipv4 addr later
	ipv4_header->dest_addr = 0xFFFFFFFF; // Broadcast; change later

	uint16_t checksum = netchecksum(packet->start, packet->start + sizeof(IPV4_HEADER));
	ipv4_header->checksum = netswap16(checksum);

	return;

}