#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "dhcp_client.h"
#include "socket.h"
#include "dhcp.h"
#include "print.h"
#include "interrupt.h"
#include "memory.h"
#include "netswap.h"
#include "serial.h"

ERR_CODE dhcp_client(NETWORK_INTERFACE* intf) {

	SOCKET* sck = ksocket(intf, BOOTP_PORT_CLIENT);
	ERR_CODE error_code = kconnect(sck, 0, BOOTP_PORT_SERVER, SOCKET_PROTOCOL_UDP);

	disable_interrupts();
	print_string("kconnect error code: ");
	print_dword(error_code);
	print_newline();
	enable_interrupts();

	disable_interrupts();
	void* send_buf = kalloc(1);
	void* recv_buf = kalloc(1);
	enable_interrupts();
	kwrite(sck, send_buf, dhcp_req(intf, send_buf, DHCP_DISCOVER, 0, 0));
		
	disable_interrupts();
 	serial_write_string("[DHCP_CLIENT] Message sent.");
 	serial_write_newline();
 	enable_interrupts();

	uint32_t recv_len = kread(sck, recv_buf);

 	disable_interrupts();
 	serial_write_string("[DHCP_CLIENT] Message received.");
 	serial_write_newline();
 	enable_interrupts();

	if (recv_len == 0) {
		disable_interrupts();
		kfree(send_buf, 1);
		kfree(recv_buf, 1);
		enable_interrupts();
		return E_FAIL;
	}

	// -- Check if it is DHCP_OFFER, get proffered ip address
	
	DHCP_HEADER* dhcp_header = (DHCP_HEADER*)recv_buf;
	void* dhcp_options = recv_buf + sizeof(DHCP_HEADER);
	uint8_t is_dhcpoffer = 0;
	uint32_t offer_dhcp = 0;
	uint32_t offer_dns = 0;
	uint32_t offer_subnet_mask = 0;
	uint32_t offer_ip = 0;

 	if (dhcp_header->op == BOOTP_BOOTREPLY && *(uint32_t*)dhcp_options == netswap32(BOOTP_MAGIC_NUMBER)) {

		dhcp_options += 4;
		while (*(uint8_t*)dhcp_options != BOOTP_OPTION_END) {
				
				switch(*(uint8_t*)dhcp_options) {
					case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
						if (*(uint8_t*)(dhcp_options + 2) == DHCP_OFFER) is_dhcpoffer = 1;
						break;
					case BOOTP_OPTION_DHCP_SERVER_IDENTIFIER:
						offer_dhcp = *(uint32_t*)(dhcp_options + 2);
						offer_dhcp = netswap32(offer_dhcp);
						break;
					case BOOTP_OPTION_DOMAIN_NAME_SERVER:
						offer_dns = *(uint32_t*)(dhcp_options + 2);
						offer_dns = netswap32(offer_dns);
						break;
					case BOOTP_OPTION_SUBNET_MASK:
						offer_subnet_mask = *(uint32_t*)(dhcp_options + 2);
						offer_subnet_mask = netswap32(offer_subnet_mask);
					default:
						break;
				}
				dhcp_options += 2 + *(uint8_t*)(dhcp_options + 1);
		}

 	}
	
	if (!is_dhcpoffer) {
		disable_interrupts();
		kfree(send_buf, 1);
		kfree(recv_buf, 1);
		enable_interrupts();
		return E_FAIL;
	}

 	disable_interrupts();
 	serial_write_string("[DHCP_CLIENT] DHCP_OFFER received.");
 	serial_write_newline();
 	enable_interrupts();

	offer_ip = netswap32(dhcp_header->yiaddr);
	kwrite(sck, send_buf, dhcp_req(intf, send_buf, DHCP_REQUEST, offer_ip, offer_dhcp));

	recv_len = kread(sck, recv_buf);

	if (recv_len == 0) {
		disable_interrupts();
		kfree(send_buf, 1);
		kfree(recv_buf, 1);
		enable_interrupts();
		return E_FAIL;
	}

	// -- Check if it is DHCP_ACK, set proffered ip address

	uint8_t is_dhcpack = 0;
	dhcp_header = (DHCP_HEADER*)recv_buf;
	dhcp_options = recv_buf + sizeof(DHCP_HEADER);

	if (dhcp_header->op == BOOTP_BOOTREPLY && *(uint32_t*)dhcp_options == netswap32(BOOTP_MAGIC_NUMBER)) {

		dhcp_options += 4;
		while (*(uint8_t*)dhcp_options != BOOTP_OPTION_END) {
				
				switch(*(uint8_t*)dhcp_options) {
					case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
						if (*(uint8_t*)(dhcp_options + 2) == DHCP_ACK) is_dhcpack = 1;
						break;
					default:
						break;
				}
				dhcp_options += 2 + *(uint8_t*)(dhcp_options + 1);
		}

 	}

 	if (!is_dhcpack) {
	 	disable_interrupts();
		kfree(send_buf, 1);
		kfree(recv_buf, 1);
		enable_interrupts();
		return E_FAIL;
	}

	// -- Set interface ip address

	intf->ip_addr = offer_ip;
	intf->ip_addr_dhcp = offer_dhcp;
	intf->ip_addr_dns = offer_dns;
	intf->subnet_mask = offer_subnet_mask;

 	disable_interrupts();
	kfree(send_buf, 1);
	kfree(recv_buf, 1);
	enable_interrupts();

	return E_OK;

}