#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "packet.h"
#include "ipv4.h"
#include "memory.h"
#include "netswap.h"
#include "print.h"
#include "checksum.h"
#include "serial.h"
#include "interrupt.h"
#include "ethernet.h"
#include "tcp.h"

typedef struct __attribute__((__packed__)) PSEUDO_IPV4_HEADER {

	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t reserved;
	uint8_t protocol;
	uint16_t tcp_len;

} PSEUDO_IPV4_HEADER;

extern uint32_t g_this_ip; 
extern SOCKET_LIST* g_sck_list;

void tcp_req(NET_PACKET* pkt, uint32_t send_port, uint32_t recv_port, uint32_t ip, uint8_t flags, uint32_t seq, uint32_t ack) {

	pkt->start -= sizeof(TCP_HEADER);
	TCP_HEADER* tcp_header = (TCP_HEADER*)pkt->start;
	memset(tcp_header, 0, sizeof(TCP_HEADER));

	tcp_header->src_port = netswap16(recv_port);
	tcp_header->dest_port = netswap16(send_port);
	tcp_header->data_offset = 5 << 4;
	tcp_header->flags = flags;
	tcp_header->window = netswap16(0x4000);
	tcp_header->seq_number = netswap32(seq);
	tcp_header->ack_number = netswap32(ack);

	// PSEUDO_IPV4_HEADER pseudo_header = {netswap32(g_this_ip), netswap32(ip), 0, IPV4_PROTOCOL_TCP, netswap16(20)};
	PSEUDO_IPV4_HEADER* pseudo_header = (PSEUDO_IPV4_HEADER*)(pkt->start - sizeof(PSEUDO_IPV4_HEADER));
	pseudo_header->src_ip = netswap32(g_this_ip);
	pseudo_header->dst_ip = netswap32(ip);
	pseudo_header->reserved = 0;
	pseudo_header->protocol = IPV4_PROTOCOL_TCP;
	pseudo_header->tcp_len = netswap16(pkt->end - pkt->start);

	uint16_t checksum = netchecksum(pkt->start - sizeof(PSEUDO_IPV4_HEADER), pkt->end);
	tcp_header->checksum = netswap16(checksum);

	ipv4_req(pkt, IPV4_PROTOCOL_TCP, ip);

	return;

}

uint8_t tcp_decode(void* buf, size_t size) {

	TCP_HEADER* tcp_header = (TCP_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_TCP_HEADER] TCP header with destination port 0x");
	serial_write_word(netswap16(tcp_header->dest_port));
	serial_write_newline();
	serial_write_string("[RX_TCP_HEADER] TCP header with source port 0x");
	serial_write_word(netswap16(tcp_header->src_port));
	serial_write_newline();
	enable_interrupts();

	// -- Check for socket open on TCP destination port

	SOCKET* sck = g_sck_list->sck_head;

	while (sck != NULL && sck->port != netswap16(tcp_header->dest_port)) sck = sck->sck_next;

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

		memcpy(recv_buffer, buf, size);

		sck->recv_buffer = recv_buffer;
		sck->recv_size = size;

		return 0;
	}

}