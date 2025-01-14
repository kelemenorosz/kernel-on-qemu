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
#include "networking.h"

typedef struct __attribute__((__packed__)) TCP_HEADER {

	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq_number;
	uint32_t ack_number;
	uint8_t data_offset;
	uint8_t flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_pointer;

} TCP_HEADER;

typedef struct __attribute__((__packed__)) PSEUDO_IPV4_HEADER {

	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t reserved;
	uint8_t protocol;
	uint16_t tcp_len;

} PSEUDO_IPV4_HEADER;

// extern uint32_t g_this_ip; 
// extern SOCKET_LIST* g_sck_list;

void tcp_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint32_t send_port, uint32_t recv_port, uint32_t ip, uint8_t flags, uint32_t seq, uint32_t ack) {

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

	PSEUDO_IPV4_HEADER* pseudo_header = (PSEUDO_IPV4_HEADER*)(pkt->start - sizeof(PSEUDO_IPV4_HEADER));
	pseudo_header->src_ip = netswap32(intf->ip_addr);
	pseudo_header->dst_ip = netswap32(ip);
	pseudo_header->reserved = 0;
	pseudo_header->protocol = IPV4_PROTOCOL_TCP;
	pseudo_header->tcp_len = netswap16(pkt->end - pkt->start);

	uint16_t checksum = netchecksum(pkt->start - sizeof(PSEUDO_IPV4_HEADER), pkt->end);
	tcp_header->checksum = netswap16(checksum);

	ipv4_req(pkt, intf, IPV4_PROTOCOL_TCP, ip);

	return;

}

ERR_CODE tcp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf, uint16_t size) {

	TCP_HEADER* tcp_header = (TCP_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_TCP_HEADER] TCP header with destination port 0x");
	serial_write_word(netswap16(tcp_header->dest_port));
	serial_write_newline();
	serial_write_string("[RX_TCP_HEADER] TCP header with source port 0x");
	serial_write_word(netswap16(tcp_header->src_port));
	serial_write_newline();
	enable_interrupts();

	uint16_t dest_port = netswap16(tcp_header->dest_port);
	uint16_t src_port = netswap16(tcp_header->src_port);

	memcpy(msg->buf, buf + sizeof(TCP_HEADER), size - sizeof(TCP_HEADER));
	msg->len = size - sizeof(TCP_HEADER);

	msg_desc->src_port = src_port;
	msg_desc->dest_port = dest_port;
	msg_desc->protocol = NETWORK_MESSAGE_PROTOCOL_TCP;
	msg_desc->flags = tcp_header->flags;
	msg_desc->seq = netswap32(tcp_header->seq_number);
	msg_desc->ack = netswap32(tcp_header->ack_number);

	return E_OK;

}