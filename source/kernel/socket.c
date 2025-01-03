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

SOCKET* ksocket(NETWORK_INTERFACE* intf, uint32_t port_in) {

	// -- Allocate memory for socket

	disable_interrupts();
	SOCKET* sck = (SOCKET*)kalloc(1);
	enable_interrupts();
	memset(sck, 0, sizeof(SOCKET));
	sck->port_in = port_in;
	sck->intf = intf;

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
		return E_FAIL;
	}

	// -- UDP

	if (protocol == SOCKET_PROTOCOL_UDP && sck->intf->ip_addr == 0 && ip_out != 0) return E_FAIL;
	if (protocol == SOCKET_PROTOCOL_UDP) {

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
	net_msg->buf = buf;
	net_msg->len = len;

	disable_interrupts();
	queue_push(sck->message_queue_out, net_msg);
	enable_interrupts();

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
