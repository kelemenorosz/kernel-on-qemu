#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "ethernet.h"
#include "print.h"
#include "packet.h"
#include "udp.h"
#include "netswap.h"
#include "interrupt.h"
#include "dhcp.h"
#include "serial.h"

typedef struct __attribute__((__packed__)) DHCP_HEADER {

	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];

} DHCP_HEADER;

#define BOOTP_BOOTREQUEST	1
#define BOOTP_BOOTREPLY		2

#define ARP_HARDWARE_TYPE_ETHERNET 1

#define BOOTP_MAGIC_NUMBER 0x63825363

#define BOOTP_OPTION_SUBNET_MASK 				1
#define BOOTP_OPTION_ROUTER 					3
#define BOOTP_OPTION_DOMAIN_NAME_SERVER 		6
#define BOOTP_OPTION_REQESTED_IP_ADDRESS 		50
#define BOOTP_OPTION_DHCP_MESSAGE_TYPE 			53
#define BOOTP_OPTION_DHCP_SERVER_IDENTIFIER		54
#define BOOTP_OPTION_PARAMETER_REQUEST_LIST 	55
#define BOOTP_OPTION_END 						255

#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPDECLINE 4
#define DHCPACK 5

#define BOOTP_PORT_CLIENT 68
#define BOOTP_PORT_SERVER 67

extern ETHERNET_DEVICE g_ethernet_device;
uint32_t g_xid = 0;
uint32_t g_dhcp_ip = 0;
uint32_t g_dns_ip = 0;
uint32_t g_this_ip = 0;
uint32_t g_subnet_mask = 0;
uint32_t g_this_ip_usable = 0;

uint8_t dhcp_client() {

	// -- Open socket on port 68
	
	SOCKET* sck = ksocket(0, 68, ETHERNET_TRANSPORT_PROTOCOL_UDP);

	// -- Build DHCP_DISCOVER

	disable_interrupts();
	void* req_buf = kalloc(1);
	enable_interrupts();

	uint32_t req_len = dhcp_req(req_buf, DHCPDISCOVER);

	// -- Send DHCP_DISCOVER

	ksend(sck, req_buf, req_len, BOOTP_PORT_SERVER, 0);
	
	// -- Receive DHCP_OFFER

	void* recv_buf = kreceive(sck);

	DHCP_HEADER* dhcpoffer_header = (DHCP_HEADER*)recv_buf;
	uint8_t* dhcpoffer_options = (uint8_t*)(recv_buf + sizeof(DHCP_HEADER));

	uint8_t is_dhcpoffer = 0;

	if (dhcpoffer_header->op == BOOTP_BOOTREPLY && *(uint32_t*)dhcpoffer_options == netswap32(BOOTP_MAGIC_NUMBER)) {
		
		disable_interrupts();
		serial_write_string("[DHCP] Received DHCP_OFFER.");
		serial_write_newline();
		enable_interrupts();

		dhcpoffer_options += 4;
		while (*dhcpoffer_options != BOOTP_OPTION_END) {
				
				switch(*dhcpoffer_options) {
					case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
						if (*(dhcpoffer_options + 2) == DHCPOFFER) is_dhcpoffer = 1;
						break;
					case BOOTP_OPTION_DHCP_SERVER_IDENTIFIER:
						g_dhcp_ip = *(uint32_t*)(dhcpoffer_options + 2);
						g_dhcp_ip = netswap32(g_dhcp_ip);
						break;
					case BOOTP_OPTION_DOMAIN_NAME_SERVER:
						g_dns_ip = *(uint32_t*)(dhcpoffer_options + 2);
						g_dns_ip = netswap32(g_dns_ip);
						break;
					case BOOTP_OPTION_SUBNET_MASK:
						g_subnet_mask = *(uint32_t*)(dhcpoffer_options + 2);
						g_subnet_mask = netswap32(g_subnet_mask);
					default:
						break;
				}
				dhcpoffer_options += 2 + *(dhcpoffer_options + 1);
		}	
	}

	if (is_dhcpoffer) {

		g_this_ip = netswap32(dhcpoffer_header->yiaddr);

	}

	// -- Build DHCP_REQUEST

	req_len = dhcp_req(req_buf, DHCPREQUEST);

	// -- Send DHCP_REQUEST

	disable_interrupts();
	serial_write_string("[DHCP] Sending DHCP_REQUEST.");
	serial_write_newline();
	enable_interrupts();

	ksend(sck, req_buf, req_len, BOOTP_PORT_SERVER, 0);	

	// -- Clear socket buffer
	sck->recv_buffer = NULL;
	disable_interrupts();
	kfree(recv_buf, 1);
	enable_interrupts();

	// -- Receive DHCP_ACK / DHCP_NAK
	recv_buf = kreceive(sck);

	DHCP_HEADER* dhcpack_header = (DHCP_HEADER*)recv_buf;
	uint8_t* dhcpack_options = (uint8_t*)(recv_buf + sizeof(DHCP_HEADER));

	uint8_t is_ack = 0;

	if (dhcpack_header->op == BOOTP_BOOTREPLY && *(uint32_t*)dhcpack_options == netswap32(BOOTP_MAGIC_NUMBER)) {
		dhcpack_options += 4;
		while (*dhcpack_options != BOOTP_OPTION_END) {
				
				switch(*dhcpack_options) {
					case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
						if (*(dhcpack_options + 2) == DHCPACK) is_ack = 1;
						break;
					default:
						break;
				}
				dhcpack_options += 2 + *(dhcpack_options + 1);
		}	
	}

	// -- Clear socket buffer
	disable_interrupts();
	kfree(recv_buf, 1);
	enable_interrupts();

	// -- TODO -- Close socket. There is not kclose()

	if (is_ack) g_this_ip_usable = 1;

	return !is_ack;

}

uint32_t dhcp_req(void* buf, uint32_t req) {

	DHCP_HEADER* dhcp_header = (DHCP_HEADER*)buf;
	memset(dhcp_header, 0, sizeof(DHCP_HEADER));

	dhcp_header->op = BOOTP_BOOTREQUEST;
	dhcp_header->htype = ARP_HARDWARE_TYPE_ETHERNET;
	dhcp_header->hlen = 6;
	dhcp_header->hops = 0;
	dhcp_header->xid = netswap32(g_xid++);
	dhcp_header->secs = 0;
	dhcp_header->flags = 0;
	memcpy((void*)&dhcp_header->chaddr[0], (void*)&g_ethernet_device.mac_addr[0], 6); 

	uint8_t* dhcp_options = (uint8_t*)(buf + sizeof(DHCP_HEADER));

	if (req == DHCPDISCOVER) {

		*(uint32_t*)dhcp_options = netswap32(BOOTP_MAGIC_NUMBER);
		dhcp_options += 4;

		*dhcp_options++ = BOOTP_OPTION_DHCP_MESSAGE_TYPE;
		*dhcp_options++ = 1;
		*dhcp_options++ = req;

		*dhcp_options++ = BOOTP_OPTION_REQESTED_IP_ADDRESS;
		*dhcp_options++ = 4;
		*dhcp_options++ = 192;
		*dhcp_options++ = 168;
		*dhcp_options++ = 1;
		*dhcp_options++ = 120;

		*dhcp_options++ = BOOTP_OPTION_PARAMETER_REQUEST_LIST;
		*dhcp_options++ = 3;
		*dhcp_options++ = BOOTP_OPTION_SUBNET_MASK;
		*dhcp_options++ = BOOTP_OPTION_ROUTER;
		*dhcp_options++ = BOOTP_OPTION_DOMAIN_NAME_SERVER;

	}
	else if (req == DHCPREQUEST) {

		dhcp_header->siaddr = netswap32(g_dhcp_ip);

		*(uint32_t*)dhcp_options = netswap32(BOOTP_MAGIC_NUMBER);
		dhcp_options += 4;

		*dhcp_options++ = BOOTP_OPTION_DHCP_MESSAGE_TYPE;
		*dhcp_options++ = 1;
		*dhcp_options++ = req;

		*dhcp_options++ = BOOTP_OPTION_REQESTED_IP_ADDRESS;
		*dhcp_options++ = 4;
		*(uint32_t*)dhcp_options = netswap32(g_this_ip);
		dhcp_options += 4;

		*dhcp_options++ = BOOTP_OPTION_DHCP_SERVER_IDENTIFIER;
		*dhcp_options++ = 4;
		*(uint32_t*)dhcp_options = netswap32(g_dhcp_ip);
		dhcp_options += 4;

	}

	*dhcp_options++ = BOOTP_OPTION_END;
	
	uint32_t msg_len = (void*)dhcp_options - buf;

	return msg_len;

}
