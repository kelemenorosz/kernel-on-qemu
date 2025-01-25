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
#include "gdt.h"
#include "virtual8086.h"
#include "exception.h"

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

	gdt_init();

	memory_init();
	interrupt_init();
	time_init();
	serial_init();
	keyboard_init();
	exception_init();

	disable_interrupts();
	print_string("Hello there!");
	print_newline();
	enable_interrupts();
	
	PCI_DEVICE_CLASS ahci_device_class = {};
	PCI_ENUM_TOKEN ahci_pci_token = {};
	
	ahci_device_class.classCode = 0x1;
	ahci_device_class.subClass = 0x6;
	ahci_device_class.progIF = 0x1;

	find_PCI_device(&ahci_pci_token, &ahci_device_class);

	disable_interrupts();
	print_string("AHCI PCI bus: ");
	print_byte(ahci_pci_token.bus);
	print_string(", device: ");
	print_byte(ahci_pci_token.device);
	print_string(", function: ");
	print_byte(ahci_pci_token.function);
	print_newline();
	enable_interrupts();

	AHCI_init(&ahci_pci_token);

	disable_interrupts();
	void* ahci_read_buf = kalloc(1);
	enable_interrupts();

	AHCI_read(0, ahci_read_buf, 0x83, 0, 1);

	disable_interrupts();
	print_dword(*((uint32_t*)ahci_read_buf));
	print_newline();
	enable_interrupts();

	memcpy((void*)0x30000, ahci_read_buf, 0x200);

	PIC_line_disable(0x0);

	virtual8086_init();

	while (true) {

		sleep(100);
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
