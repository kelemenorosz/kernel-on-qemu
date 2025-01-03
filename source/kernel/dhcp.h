#ifndef DHCP_H
#define DHCP_H

#include "packet.h"
#include "networking.h"

#define BOOTP_PORT_CLIENT 68
#define BOOTP_PORT_SERVER 67

#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5

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

uint32_t dhcp_req(NETWORK_INTERFACE* intf, void* buf, uint32_t req, uint32_t ip_addr, uint32_t dhcp_addr);

#endif /* DHCP_H */












// uint8_t dhcp_client();