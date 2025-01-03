#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "networking.h"
#include "intel_8254xx.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "scheduler.h"
#include "time.h"
#include "list.h"
#include "socket.h"
#include "packet.h"
#include "udp.h"
#include "serial.h"

LIST* g_network_interfaces = NULL;

void networking_init() {

	// -- Allocate memory for network interface list

	disable_interrupts();
	g_network_interfaces = (LIST*)kalloc(1);
	enable_interrupts();

	// -- Initialize network interface list

	g_network_interfaces->head = NULL;

	// -- Start networking tx poll process

	task_create(networking_tx_poll, "Networking TX Poll");
	
	return;

}

ERR_CODE networking_init_device(PCI_ENUM_TOKEN* pci_token) {

	// -- Check for correct device class

	uint32_t device_class = get_PCI_offset_token(pci_token, 0x8);
	if (!((uint8_t)(device_class >> 0x18) == 0x2 && (uint8_t)(device_class >> 0x10) == 0x0)) {
		return E_FAIL;
	}

	// -- Get vendor and device ID

	uint32_t device_vendor_id = get_PCI_offset_token(pci_token, 0x0);
	if ((uint16_t)(device_vendor_id >> 0x10) == 0x100E && (uint16_t)(device_vendor_id) == 0x8086) {
		return intel_8254xx_init(pci_token);
	}
	else {
		return E_FAIL;
	}

}

void networking_enum_interfaces() {

	LIST_ENTRY* list_entry = g_network_interfaces->head;
	NETWORK_INTERFACE* intf = NULL;
	size_t intf_index = 0;
	while (list_entry != NULL) {
		intf = (NETWORK_INTERFACE*)list_entry->ptr;
		print_string("Interface ");
		print_byte((uint8_t)intf_index);
		print_string(" ether addr: ");
		for (size_t i = 0; i < 6; ++i) {
			print_byte(intf->ether_addr[i]);
			if (i != 5) print_string(":");
		}
		print_newline();
		list_entry = list_entry->next;
		intf_index++;
	}

	return;

}

NETWORK_INTERFACE* get_network_interface(uint32_t index) {

	LIST_ENTRY* list_entry = g_network_interfaces->head;
	while (index != 0  && list_entry != NULL) {
		list_entry = list_entry->next;
		index--;
	}

	if (list_entry == NULL) {
		return NULL;
	}

	return (NETWORK_INTERFACE*)list_entry->ptr;

}

void networking_tx_poll() {

	while (true) {

		// -- Iterate through all processes in g_processes. The list doesn't include the idle process.

		LIST_ENTRY* process_entry = g_processes->head;
		while (process_entry != NULL) {
			TASK_STATE* task_state = (TASK_STATE*)process_entry->ptr;

			// -- Iterate through all sockets of the process

			disable_interrupts();
			serial_write_string("[TX_POLL] Checking for sockets for process with ESP 0x");
			serial_write_dword(task_state->esp);
			serial_write_newline();
			enable_interrupts();

			LIST_ENTRY* sck_entry = NULL;
			if (task_state->sockets != NULL) sck_entry = task_state->sockets->head;
			while (sck_entry != NULL) {

				disable_interrupts();
				serial_write_string("[TX_POLL] Socket found for process with ESP 0x");
				serial_write_dword(task_state->esp);
				serial_write_newline();
				enable_interrupts();

				// -- Send messages until the outbound message queue is empty 

				SOCKET* sck = (SOCKET*)sck_entry->ptr;
				while (sck->message_queue_out->head != NULL) {
				
					disable_interrupts();
					NETWORK_MESSAGE* net_msg = queue_pop(sck->message_queue_out);
					void* wrap_buf = kalloc(1); // TODO: Make sure to kfree() in tx_send() when inserting new buffer into TX descriptor  
					enable_interrupts();
					memcpy(wrap_buf + 0x100, net_msg->buf, net_msg->len);
					NETWORK_PACKET pkt = {wrap_buf + 0x100, wrap_buf + 0x100 + net_msg->len};
					disable_interrupts();
					kfree(net_msg->buf, 1);
					kfree(net_msg, 1);
					enable_interrupts();

					// -- Assemble packet
					if (sck->protocol == SOCKET_PROTOCOL_UDP) {
						// -- UDP
						udp_req(&pkt, sck->intf, sck->port_out, sck->port_in, sck->ip_out);
					}
					else {
						// -- TCP
					}

					// -- Send packet

					disable_interrupts();
					serial_write_string("[TX_POLL] Sending packet for process with ESP 0x");
					serial_write_dword(task_state->esp);
					serial_write_newline();
					enable_interrupts();

					sck->intf->tx_send(sck->intf->device, &pkt);
				
				}

				sck_entry = sck_entry->next;
			}

			process_entry = process_entry->next; 
		}

		sleep(100);

	}

	return;

}

void networking_rx_sort(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* desc) {

	// disable_interrupts();
	// serial_write_string("[RX_SORT] Processing message.");
	// serial_write_newline();
	// enable_interrupts();

	if (desc->protocol == NETWORK_MESSAGE_PROTOCOL_UDP) {

		// -- Check if there is a socket that matches the message's source and destination ports

		disable_interrupts();
		serial_write_string("[RX_SORT] Processing UDP message. Source port 0x");
		serial_write_dword(desc->src_port);
		serial_write_string(". Destination port 0x");
		serial_write_dword(desc->dest_port);
		serial_write_newline();
		enable_interrupts();

		LIST_ENTRY* process_entry = g_processes->head;
		while (process_entry != NULL) {
			
			TASK_STATE* task_state = (TASK_STATE*)process_entry->ptr;

			LIST_ENTRY* sck_entry = NULL;
			if (task_state->sockets != NULL) sck_entry = task_state->sockets->head;
			while (sck_entry != NULL) {
			
				SOCKET* sck = (SOCKET*)sck_entry->ptr;
				if (sck->port_in == desc->dest_port && sck->port_out == desc->src_port) {
					disable_interrupts();
					queue_push(sck->message_queue_in, msg);
					enable_interrupts();
					return;
				}

				sck_entry = sck_entry->next;			
			}
			
			process_entry = process_entry->next;
		}

	}
	else {

		disable_interrupts();
		kfree(msg->buf, 1);
		kfree(msg, 1);
		enable_interrupts();
	
	}

	return;

}
