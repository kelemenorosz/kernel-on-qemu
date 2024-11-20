#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "ethernet.h"
#include "print.h"
#include "packet.h"
#include "udp.h"
#include "netswap.h"

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

#define BOOTP_OPTION_DHCP_MESSAGE_TYPE 53
#define BOOTP_OPTION_REQESTED_IP_ADDRESS 50
#define BOOTP_OPTION_PARAMETER_REQUEST_LIST 55
#define BOOTP_OPTION_END 255

#define DHCPDISCOVER 1

#define BOOTP_PORT_CLIENT 68
#define BOOTP_PORT_SERVER 67

void req_dhcp(ETHERNET_DEVICE* ethernet_device, NET_PACKET* packet) {

	DHCP_HEADER* dhcp_header = (DHCP_HEADER*)packet->end;
	memset(dhcp_header, 0, sizeof(DHCP_HEADER));

	dhcp_header->op = BOOTP_BOOTREQUEST;
	dhcp_header->htype = ARP_HARDWARE_TYPE_ETHERNET;
	dhcp_header->hlen = 6; // MAC address length in bytes
	dhcp_header->hops = 0;
	dhcp_header->xid = 0x349A;
	dhcp_header->secs = 0;
	dhcp_header->flags = 0;
	memcpy((void*)&dhcp_header->chaddr[0], (void*)&ethernet_device->mac_addr[0], 6); 

	// for (uint8_t i = 0; i < 6; ++i) {
	
	// 	print_string("DHCP header chaddr: ");
	// 	print_byte(dhcp_header->chaddr[i]);
	// 	print_newline();
	
	// }

	uint8_t* options = (uint8_t*)(packet->end + sizeof(DHCP_HEADER)); 

	*(uint32_t*)options = netswap32(BOOTP_MAGIC_NUMBER);
	options += 4;

	*options = BOOTP_OPTION_DHCP_MESSAGE_TYPE;
	options++;
	*options = 1; // Option length
	options++;
	*options = DHCPDISCOVER;
	options++;

	*options = BOOTP_OPTION_REQESTED_IP_ADDRESS;
	options++;
	*options = 4; // Option length
	options++;
	*options = 192;
	*(options + 1) = 168;
	*(options + 2) = 1;
	*(options + 3) = 120;
	options += 4;

	*options = BOOTP_OPTION_PARAMETER_REQUEST_LIST;
	options++;
	*options = 3;
	options++;
	*options = 1;
	options++;
	*options = 3;
	options++;
	*options = 6;
	options++;

	*options = BOOTP_OPTION_END;
	options++;

	packet->end = (void*)options;

	req_udp(ethernet_device, packet, BOOTP_PORT_CLIENT, BOOTP_PORT_SERVER);

	return;

}
