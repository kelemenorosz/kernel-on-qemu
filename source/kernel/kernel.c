#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "io.h"
#include "print.h"
#include "interrupt.h"
#include "time.h"
#include "vga.h"
#include "pci.h"
#include "memory.h"
#include "ahci.h"
#include "cpuid.h"
#include "msr.h"
#include "mtrr.h"
#include "keyboard.h"
#include "ethernet.h"
#include "scheduler.h"
#include "eflags.h"
#include "serial.h"
#include "string.h"
#include "networking.h"
#include "err.h"
#include "socket.h"
#include "dhcp_client.h"

const size_t VGA_WIDTH = 0x50;
const size_t VGA_HEIGHT = 0x19;

size_t g_consoleIndex = 0x0;
size_t g_consoleRow = 0x0;
uint16_t* const g_VGABuffer = (uint16_t*)0xB8000;

uint32_t* const g_memoryMapPtr = (uint32_t*)0x60000;

void second_task(PPARAM value);
void third_task();

void kernel_main() {
	
	cls();

	memory_init();
	interrupt_init();
	serial_init();
	time_init();
	keyboard_init();
	scheduler_init();

	disable_interrupts();
	scheduler_start();
	enable_interrupts();

	PCI_DEVICE_CLASS ethernet_pci_class = {};
	PCI_ENUM_TOKEN ethernet_pci_token = {};

	ethernet_pci_class.classCode = 0x2;
	ethernet_pci_class.subClass = 0x0;
	ethernet_pci_class.progIF = 0x0;

	find_PCI_device_no_progif(&ethernet_pci_token, &ethernet_pci_class);

	disable_interrupts();
	print_string("Ethernet card PCI bus: ");
	print_byte(ethernet_pci_token.bus);
	print_string(", device: ");
	print_byte(ethernet_pci_token.device);
	print_string(", function: ");
	print_byte(ethernet_pci_token.function);
	print_newline();
	enable_interrupts();

	networking_init();
	ERR_CODE error_code = networking_init_device(&ethernet_pci_token);
	disable_interrupts();
	print_string("networking_init_device error code: ");
	print_dword((uint32_t)error_code);
	print_newline();
	enable_interrupts();

	// -- Get network interface
	NETWORK_INTERFACE* net_intf = get_network_interface(0);

	// -- DHCP client
	error_code = dhcp_client(net_intf);
	disable_interrupts();
	print_string("dhcp_client error code: ");
	print_dword((uint32_t)error_code);
	print_newline();
	enable_interrupts();

	// -- Connect to web server

	uint32_t webserver_ip = 0xC0A80168;

	SOCKET* web_sck = ksocket(net_intf, 80);
	error_code = kconnect(web_sck, webserver_ip, 4800, SOCKET_PROTOCOL_TCP);
	disable_interrupts();
	print_string("webserver socket kconnect error code: ");
	print_dword((uint32_t)error_code);
	print_newline();
	enable_interrupts();

	disable_interrupts();
	print_string("subnet mask: ");
	print_dword(net_intf->subnet_mask);
	print_newline();	
	enable_interrupts();

	while (true) {

		sleep(1000);
		*(g_VGABuffer + VGA_WIDTH + 0x4F) += 1;

	}

	return;

}

void second_task(PPARAM value) {

	uint32_t value_u32 = (uint32_t)value;

	while (true) {

		disable_interrupts();
		print_string("Second task! ");
		print_dword(value_u32);
		print_newline();
		enable_interrupts();

		sleep(500);

	}

	return;

}

void third_task() {

	while (true) {

		disable_interrupts();
		print_string("Third task?");
		print_newline();
		enable_interrupts();

		sleep(250);

	}

	return;

}
