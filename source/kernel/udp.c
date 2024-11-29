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

extern SOCKET_LIST* g_sck_list;

void udp_req(NET_PACKET* pkt, uint32_t send_port, uint32_t recv_port, uint32_t ip) {

	pkt->start -= sizeof(UDP_HEADER);
	UDP_HEADER* udp_header = (UDP_HEADER*)(pkt->start);
	udp_header->src_port = netswap16(recv_port);
	udp_header->dest_port = netswap16(send_port);
	udp_header->length = netswap16(pkt->end - pkt->start);

	ipv4_req(pkt, IPV4_PROTOCOL_UDP, ip);

	return;

}

uint32_t udp_decode(void* buf) {
	
	UDP_HEADER* udp_header = (UDP_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_UDP_HEADER] UDP header with destination port 0x");
	serial_write_word(netswap16(udp_header->dest_port));
	serial_write_newline();
	serial_write_string("[RX_UDP_HEADER] UDP header with source port 0x");
	serial_write_word(netswap16(udp_header->src_port));
	serial_write_newline();
	enable_interrupts();

	uint16_t dst_port = netswap16(udp_header->dest_port);

	SOCKET* sck = g_sck_list->sck_head;

	while (sck != NULL && sck->port != dst_port) sck = sck->sck_next;

	if (sck == NULL) {
		return 0;
	}
	else if (sck->recv_buffer != NULL) {
		return 1;
	}
	else {

		disable_interrupts();
		void* recv_buffer = kalloc(1);
		enable_interrupts();
		memset(recv_buffer, 0, 0x1000);
		memcpy(recv_buffer, buf + sizeof(UDP_HEADER), netswap16(udp_header->length) - sizeof(UDP_HEADER));

		sck->recv_buffer = recv_buffer;
		return 0;

	}

}