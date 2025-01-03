// #include <stddef.h>
// #include <stdint.h>
// #include <stdbool.h>

// #include "interrupt.h"
// #include "memory.h"
// #include "ethernet.h"
// #include "packet.h"
// #include "netswap.h"
// #include "serial.h"

// typedef struct __attribute__((__packed__)) ARP_HEADER {

// 	uint16_t hardware_type;
// 	uint16_t protocol_type;
// 	uint8_t hardware_size;
// 	uint8_t protocol_size;
// 	uint16_t opcode;
// 	uint8_t hardware_sender_addr[6];
// 	uint32_t protocol_sender_addr;
// 	uint8_t hardware_target_addr[6];
// 	uint32_t protocol_target_addr;

// } ARP_HEADER;

// #define ARP_HARDWARE_TYPE_ETHERNET 1

// #define ARP_PROTOCOL_TYPE_IPV4 0x0800

// #define ARP_OPCODE_REQUEST 1
// #define ARP_OPCODE_REPLY 2

// typedef struct __attribute__((__packed__)) ARP_ENTRY {

// 	uint32_t ip;
// 	uint8_t eth[8];
// 	struct ARP_ENTRY* arp_next;

// } ARP_ENTRY;

// typedef struct __attribute__((__packed__)) ARP_LIST {

// 	ARP_ENTRY* arp_head;

// } ARP_LIST;

// ARP_LIST* g_arp_list = NULL; 

// extern uint32_t g_this_ip;
// extern ETHERNET_DEVICE g_ethernet_device;

// void* arp_lookup(uint32_t ip) {

// 	// -- If arp table is not initialized initialize it
// 	if (g_arp_list == NULL) {
// 		disable_interrupts();
// 		g_arp_list = (ARP_LIST*)kalloc(1);
// 		enable_interrupts();
// 		g_arp_list->arp_head = NULL;
// 		return NULL;
// 	}

// 	// -- If arp table is initialized look for ip
// 	ARP_ENTRY* arp_entry = g_arp_list->arp_head;
// 	while (arp_entry != NULL && arp_entry->ip != ip) arp_entry = arp_entry->arp_next;

// 	if (arp_entry == NULL) {
// 		return NULL;
// 	}

// 	return &arp_entry->eth[0];

// }

// void arp_req(uint32_t ip) {

// 	disable_interrupts();
// 	void* buf = kalloc(1);
// 	enable_interrupts();

// 	ARP_HEADER* arp_header = (ARP_HEADER*)(buf + 0x100);

// 	arp_header->hardware_type = netswap16(ARP_HARDWARE_TYPE_ETHERNET);
// 	arp_header->protocol_type = netswap16(ARP_PROTOCOL_TYPE_IPV4);
// 	arp_header->hardware_size = 6;
// 	arp_header->protocol_size = 4;
// 	arp_header->opcode = netswap16(ARP_OPCODE_REQUEST);
// 	arp_header->protocol_sender_addr = netswap32(g_this_ip);
// 	arp_header->protocol_target_addr = netswap32(ip);

// 	memcpy(&arp_header->hardware_sender_addr[0], &g_ethernet_device.mac_addr[0], 6);
// 	memset(&arp_header->hardware_target_addr[0], 0x0, 6);

// 	NET_PACKET pkt = {buf + 0x100, buf + 0x100 + sizeof(ARP_HEADER)};

// 	ether_req(&pkt, ETHER_TYPE_ARP, 0);

// 	tx_send(pkt.start, pkt.end - pkt.start);

// 	return;

// }

// uint8_t arp_decode(void* buf) {

// 	ARP_HEADER* arp_header = (ARP_HEADER*)buf;
// 	uint8_t* addr = NULL;

// 	disable_interrupts();
// 	serial_write_string("[RX_ARP_HEADER] ARP Header with sender IP: ");
// 	addr = (uint8_t*)&arp_header->protocol_sender_addr;
// 	for (uint8_t i = 0; i < 4; ++i) {
// 		serial_write_byte(addr[i]);
// 		if (i != 3) serial_write_string(".");
// 	}
// 	serial_write_newline();
// 	serial_write_string("[RX_ARP_HEADER] ARP Header with target IP: ");
// 	addr = (uint8_t*)&arp_header->protocol_target_addr;
// 	for (uint8_t i = 0; i < 4; ++i) {
// 		serial_write_byte(addr[i]);
// 		if (i != 3) serial_write_string(".");
// 	}
// 	serial_write_newline();
// 	serial_write_string("[RX_ARP_HEADER] ARP Header with opcode 0x");
// 	serial_write_word(netswap16(arp_header->opcode));
// 	serial_write_newline();
// 	enable_interrupts();

// 	if (netswap16(arp_header->opcode) == ARP_OPCODE_REPLY) {

// 		// -- Check if not already in ARP list
// 		ARP_ENTRY* arp_entry = g_arp_list->arp_head;
// 		while (arp_entry != NULL && arp_entry->ip != netswap32(arp_header->protocol_sender_addr)) arp_entry = arp_entry->arp_next;

// 		if (arp_entry != NULL) return 0;

// 		// -- Save MAC in ARP list

// 		disable_interrupts();
// 		ARP_ENTRY* new_entry = (ARP_ENTRY*)kalloc(1);
// 		enable_interrupts();

// 		new_entry->ip = netswap32(arp_header->protocol_sender_addr);
// 		memcpy((void*)&new_entry->eth[0], (void*)&arp_header->hardware_sender_addr[0], 6);

// 		new_entry->arp_next = g_arp_list->arp_head;
// 		g_arp_list->arp_head = new_entry;

// 		// -- Add MAC to the receive address registers

// 		rar_add((void*)&new_entry->eth[0]);

// 		return 0;

// 	}

// 	return 0;

// }
