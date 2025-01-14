#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "ethernet.h"
#include "packet.h"
#include "netswap.h"
#include "ipv4.h"
#include "serial.h"
#include "interrupt.h"
#include "print.h"

typedef struct __attribute__((__packed__)) UDP_HEADER {

	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum; 

} UDP_HEADER;

void udp_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint32_t send_port, uint32_t recv_port, uint32_t ip) {

	pkt->start -= sizeof(UDP_HEADER);
	UDP_HEADER* udp_header = (UDP_HEADER*)(pkt->start);
	udp_header->src_port = netswap16(recv_port);
	udp_header->dest_port = netswap16(send_port);
	udp_header->length = netswap16(pkt->end - pkt->start);

	ipv4_req(pkt, intf, IPV4_PROTOCOL_UDP, ip);

	return;

}

ERR_CODE udp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf) {

	UDP_HEADER* udp_header = (UDP_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_UDP_HEADER] UDP header with destination port 0x");
	serial_write_word(netswap16(udp_header->dest_port));
	serial_write_newline();
	serial_write_string("[RX_UDP_HEADER] UDP header with source port 0x");
	serial_write_word(netswap16(udp_header->src_port));
	serial_write_newline();
	enable_interrupts();

	uint16_t dest_port = netswap16(udp_header->dest_port);
	uint16_t src_port = netswap16(udp_header->src_port);

	memcpy(msg->buf, buf + sizeof(UDP_HEADER), netswap16(udp_header->length));
	msg->len = netswap16(udp_header->length);

	msg_desc->src_port = src_port;
	msg_desc->dest_port = dest_port;
	msg_desc->protocol = NETWORK_MESSAGE_PROTOCOL_UDP;

	return E_OK;

}

