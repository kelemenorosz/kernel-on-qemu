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

const size_t VGA_WIDTH = 0x50;
const size_t VGA_HEIGHT = 0x19;

size_t g_consoleIndex = 0x0;
size_t g_consoleRow = 0x0;
uint16_t* const g_VGABuffer = (uint16_t*)0xB8000;

uint32_t* const g_memoryMapPtr = (uint32_t*)0x60000;

void second_task();
void third_task();

void kernel_main() {
	
	cls();

	memory_init();
	interrupt_init();
	time_init();
	keyboard_init();
	scheduler_init();

	// print_memmap();

	task_create(second_task);
	task_create(third_task);

	disable_interrupts();
	scheduler_start();
	enable_interrupts();

	while (true) {

		disable_interrupts();
		print_string("First task: ");
		//print_dword(get_total_ticks());
		print_newline();
		enable_interrupts();

		sleep(1000);
		*(g_VGABuffer + VGA_WIDTH + 0x4F) += 1;

	}

	return;

}

void second_task() {

	// disable_interrupts();
	// print_task_state_list();
	// enable_interrupts();

	while (true) {

		disable_interrupts();
		print_string("Second task!");
		//print_dword(get_total_ticks());
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
		//print_dword(get_total_ticks());
		print_newline();
		enable_interrupts();

		sleep(250);

	}

	return;

}
