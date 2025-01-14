#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "socket.h"
#include "memory.h"
#include "interrupt.h"
#include "scheduler.h"
#include "list.h"
#include "queue.h"
#include "time.h"
#include "serial.h"
#include "print.h"
#include "arp.h"

SOCKET* ksocket(NETWORK_INTERFACE* intf, uint32_t port_in) {

	// -- Check for and create ARP socket

	if (g_current_task_state->arp_sck == NULL) {

		disable_interrupts();
		g_current_task_state->arp_sck = (SOCKET*)kalloc(1);
		enable_interrupts();

		memset(g_current_task_state->arp_sck, 0, sizeof(SOCKET));
		g_current_task_state->arp_sck->intf = intf;
		
		disable_interrupts();
		g_current_task_state->arp_sck->message_queue_in = (QUEUE*)kalloc(1);
		g_current_task_state->arp_sck->message_queue_out = (QUEUE*)kalloc(1);
		enable_interrupts();

		g_current_task_state->arp_sck->message_queue_in->head = NULL;
		g_current_task_state->arp_sck->message_queue_in->tail = NULL;
		g_current_task_state->arp_sck->message_queue_out->head = NULL;
		g_current_task_state->arp_sck->message_queue_out->tail = NULL;

	}

	// -- Allocate memory for socket

	disable_interrupts();
	SOCKET* sck = (SOCKET*)kalloc(1);
	enable_interrupts();
	memset(sck, 0, sizeof(SOCKET));
	sck->port_in = port_in;
	sck->intf = intf;
	sck->message_queue_in = NULL;
	sck->message_queue_out = NULL;
	sck->status = S_NULL;
	sck->connect_status = S_NULL;

	// -- Add socket to process' socket list

	disable_interrupts();
	list_add(g_current_task_state->sockets, sck);
	enable_interrupts();

	disable_interrupts();
	serial_write_string("[SOCKET] ksocket() - PORT_IN 0x");
	serial_write_dword(port_in);
	serial_write_newline();
	enable_interrupts();

	return sck;

}

ERR_CODE kconnect(SOCKET* sck, uint32_t ip_out, uint32_t port_out, uint32_t protocol) {

	// -- TCP

	if (protocol == SOCKET_PROTOCOL_TCP && sck->intf->ip_addr == 0) return E_FAIL;
	if (protocol == SOCKET_PROTOCOL_TCP) {
		
		// -- 'ip_out' is on the subnet

		if ((sck->intf->subnet_mask & sck->intf->ip_addr) == (sck->intf->subnet_mask & ip_out)) {

			// -- ARP if not in cache

			ERR_CODE error_code = arp_lookup(ip_out, NULL);

			// -- If ether address is not in cache
			// TODO: error_code == E_FAIL or error_code == E_OK may fail. This may be because of enum bullshit

			if (error_code == E_FAIL)  {

				disable_interrupts();
				void* buf = kalloc(1);
				enable_interrupts();

				uint32_t buf_len = arp_req(sck->intf, buf, ip_out);
				kwrite(g_current_task_state->arp_sck, buf, buf_len);

				disable_interrupts();
				kfree(buf, 1);
				enable_interrupts();

				while (arp_lookup(ip_out, NULL) == E_FAIL) sleep(1);

				disable_interrupts();
				print_string("ip_out is in ARP cache");
				print_newline();
				enable_interrupts();

			}

		}

		// -- Set socket info

		sck->ip_out = ip_out;
		sck->port_out = port_out;
		sck->protocol = protocol;
		disable_interrupts();
		sck->message_queue_in = (QUEUE*)kalloc(1);
		sck->message_queue_out = (QUEUE*)kalloc(1);
		enable_interrupts();
		sck->message_queue_in->head = NULL;
		sck->message_queue_in->tail = NULL;
		sck->message_queue_out->head = NULL;
		sck->message_queue_out->tail = NULL;
		sck->seq = 0;
		sck->ack = 0;
		sck->status = S_SYN;
		sck->connect_status = S_SYN;

		disable_interrupts();
		serial_write_string("[SOCKET] kconnect() - PORT_IN 0x");
		serial_write_dword(sck->port_in);
		serial_write_string("; PORT_OUT 0x");
		serial_write_dword(sck->port_out);
		serial_write_newline();
		enable_interrupts();

		// -- Wait for SYN-ACK handshake

		while (sck->connect_status != S_OPEN) sleep(2);

		return E_OK;
	
	}

	// -- UDP

	if (protocol == SOCKET_PROTOCOL_UDP && sck->intf->ip_addr == 0 && ip_out != 0) return E_FAIL;
	if (protocol == SOCKET_PROTOCOL_UDP) {

		sck->ip_out = ip_out;
		sck->port_out = port_out;
		sck->protocol = protocol;
		sck->seq = 0;
		sck->ack = 0;
		disable_interrupts();
		sck->message_queue_in = (QUEUE*)kalloc(1);
		sck->message_queue_out = (QUEUE*)kalloc(1);
		enable_interrupts();

		sck->message_queue_in->head = NULL;
		sck->message_queue_in->tail = NULL;
		sck->message_queue_out->head = NULL;
		sck->message_queue_out->tail = NULL;

		disable_interrupts();
		serial_write_string("[SOCKET] kconnect() - PORT_IN 0x");
		serial_write_dword(sck->port_in);
		serial_write_string("; PORT_OUT 0x");
		serial_write_dword(sck->port_out);
		serial_write_newline();
		enable_interrupts();

		return E_OK;
	
	}

	return E_FAIL;

}

ERR_CODE kwrite(SOCKET* sck, void* buf, size_t len) {

	disable_interrupts();
	serial_write_string("[SOCKET] kwrite() - PORT_IN 0x");
	serial_write_dword(sck->port_in);
	serial_write_string("; PORT_OUT 0x");
	serial_write_dword(sck->port_out);
	serial_write_newline();
	enable_interrupts();


	disable_interrupts();
	void* copy_buf = kalloc(1);
	NETWORK_MESSAGE* net_msg = (NETWORK_MESSAGE*)kalloc(1); 
	enable_interrupts();
	memcpy(copy_buf, buf, 0x1000);
	net_msg->buf = copy_buf;
	net_msg->len = len;

	disable_interrupts();
	queue_push(sck->message_queue_out, net_msg);
	enable_interrupts();

	sck->status = S_WRITE;
	
	return E_OK;

}

uint32_t kread(SOCKET* sck, void* buf) {

	while (sck->message_queue_in->head == NULL) sleep(10);
	disable_interrupts();
	NETWORK_MESSAGE* net_msg = queue_pop(sck->message_queue_in); 	
	enable_interrupts();

	memcpy(buf, net_msg->buf, net_msg->len);
	uint32_t message_len = net_msg->len;

	disable_interrupts();
	kfree(net_msg->buf, 1);
	kfree(net_msg, 1);
	enable_interrupts();

	disable_interrupts();
	serial_write_string("[SOCKET] kread() - PORT_IN 0x");
	serial_write_dword(sck->port_in);
	serial_write_string("; PORT_OUT 0x");
	serial_write_dword(sck->port_out);
	serial_write_newline();
	enable_interrupts();

	return message_len;

}

void kclose(SOCKET* sck) {

	disable_interrupts();
	serial_write_string("[SOCKET] kclose() - PORT_IN 0x");
	serial_write_dword(sck->port_in);
	serial_write_string("; PORT_OUT 0x");
	serial_write_dword(sck->port_out);
	serial_write_newline();
	enable_interrupts();

	if (sck->protocol == SOCKET_PROTOCOL_TCP) {
		// -- TCP
		while (sck->status != S_OPEN) sleep(2);
		disable_interrupts();
		sck->status = S_FIN;
		enable_interrupts();
	}
	else if (sck->protocol == SOCKET_PROTOCOL_UDP) {
		// -- UDP
	}

	return;

} 