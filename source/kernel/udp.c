#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ethernet.h"
#include "packet.h"
#include "netswap.h"
#include "ipv4.h"

typedef struct __attribute__((__packed__)) UDP_HEADER {

	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum; 

} UDP_HEADER;

void req_udp(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet, uint16_t src_port, uint16_t dest_port) {

	packet->start -= sizeof(UDP_HEADER);
	UDP_HEADER* udp_header = (UDP_HEADER*)packet->start;

	udp_header->src_port = netswap16(src_port);
	udp_header->dest_port = netswap16(dest_port);
	udp_header->length = netswap16(packet->end - packet->start);
	udp_header->checksum = 0;

	req_ipv4(ethernet_device, packet, IPV4_PROTOCOL_UDP);

	return;

}