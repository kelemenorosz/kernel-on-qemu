#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "memory.h"
#include "ethernet.h"
#include "packet.h"
#include "netswap.h"
#include "serial.h"

typedef struct __attribute__((__packed__)) ARP_HEADER {

	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_size;
	uint8_t protocol_size;
	uint16_t opcode;
	uint8_t hardware_sender_addr[6];
	uint32_t protocol_sender_addr;
	uint8_t hardware_target_addr[6];
	uint32_t protocol_target_addr;

} ARP_HEADER;

#define ARP_HARDWARE_TYPE_ETHERNET 1

#define ARP_PROTOCOL_TYPE_IPV4 0x0800

#define ARP_OPCODE_REQUEST 1
#define ARP_OPCODE_REPLY 2

typedef struct __attribute__((__packed__)) ARP_ENTRY {

	uint32_t ip;
	uint8_t eth[8];
	struct ARP_ENTRY* arp_next;

} ARP_ENTRY;

typedef struct __attribute__((__packed__)) ARP_LIST {

	ARP_ENTRY* arp_head;

} ARP_LIST;

ARP_LIST* g_arp_list = NULL; 

ERR_CODE arp_lookup(uint32_t ip, uint8_t* eth_addr) {

	// -- If arp table is not initialized initialize it

	if (g_arp_list == NULL) {
		disable_interrupts();
		g_arp_list = (ARP_LIST*)kalloc(1);
		enable_interrupts();
		g_arp_list->arp_head = NULL;
		return E_FAIL;
	}

	// -- If arp table is initialized look for ip

	ARP_ENTRY* arp_entry = g_arp_list->arp_head;
	while (arp_entry != NULL && arp_entry->ip != ip) arp_entry = arp_entry->arp_next;

	if (arp_entry == NULL) {
		return E_FAIL;
	}

	if (eth_addr != NULL) memcpy(eth_addr, &arp_entry->eth[0], 6);
	return E_OK;

}

uint32_t arp_req(NETWORK_INTERFACE* intf, void* buf, uint32_t ip) {

	ARP_HEADER* arp_header = (ARP_HEADER*)buf;

	arp_header->hardware_type = netswap16(ARP_HARDWARE_TYPE_ETHERNET);
	arp_header->protocol_type = netswap16(ARP_PROTOCOL_TYPE_IPV4);
	arp_header->hardware_size = 6;
	arp_header->protocol_size = 4;
	arp_header->opcode = netswap16(ARP_OPCODE_REQUEST);
	arp_header->protocol_sender_addr = netswap32(intf->ip_addr);
	arp_header->protocol_target_addr = netswap32(ip);

	memcpy(&arp_header->hardware_sender_addr[0], &intf->ether_addr[0], 6);
	memset(&arp_header->hardware_target_addr[0], 0x0, 6);

	return sizeof(ARP_HEADER);

}

ERR_CODE arp_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf) {

	msg_desc->protocol = NETWORK_MESSAGE_PROTOCOL_ARP;
	msg_desc->src_port = 0;
	msg_desc->dest_port = 0;

	memcpy(msg->buf, buf, sizeof(ARP_HEADER));
	msg->len = sizeof(ARP_HEADER);

	return E_OK;

}

void arp_sort(NETWORK_MESSAGE* msg, void* device, void(*rar_add)(void* device, void* eth_addr)) {

	ARP_HEADER* arp_header = (ARP_HEADER*)msg->buf;

	if (netswap16(arp_header->opcode) == ARP_OPCODE_REPLY) {

		// -- Check if not already in ARP list
		
		ARP_ENTRY* arp_entry = g_arp_list->arp_head;
		while (arp_entry != NULL && arp_entry->ip != netswap32(arp_header->protocol_sender_addr)) arp_entry = arp_entry->arp_next;

		if (arp_entry != NULL) return;

		// -- Save MAC in ARP list

		disable_interrupts();
		ARP_ENTRY* new_entry = (ARP_ENTRY*)kalloc(1);
		enable_interrupts();

		new_entry->ip = netswap32(arp_header->protocol_sender_addr);
		memcpy((void*)&new_entry->eth[0], (void*)&arp_header->hardware_sender_addr[0], 6);

		new_entry->arp_next = g_arp_list->arp_head;
		g_arp_list->arp_head = new_entry;

		// -- Add MAC to the receive address registers

		rar_add(device, (void*)&new_entry->eth[0]);

	}

	return;

}
