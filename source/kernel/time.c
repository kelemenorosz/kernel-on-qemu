#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "time.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "memory.h"
#include "serial.h"

#define PIT_RELOAD_VALUE 1193
#define NANOSECONDS_BETWEEN_TICKS 999847
#define TIME_SLICE_IN_TICKS 2

uint32_t g_PIT_count_down = 0x0;
uint32_t g_scheduler_time = TIME_SLICE_IN_TICKS;
uint32_t g_total_ticks = 0x0;

DELTA_QUEUE* g_delta_queue = NULL;

void PIT_init();
void interrupt_function_PIT();
extern void interrupt_wrapper_PIT();
extern void interrupt_wrapper_software_blocking();

extern TASK_STATE* g_current_task_state;

void print_delta_queue();

/*
Function: 		sleep
Description: 	Should only be called after scheduler.h has been initialized.
				Inserts the current running task into the blocking delta queue.
				Raises a software interrupt, which then performs a task switch.
Return:			NONE
*/
void sleep(uint32_t ticks_to_sleep) {

	disable_interrupts();
	
	// Set task to blocking
	g_current_task_state->is_blocking = 0x1;

	// Create delta queue element
	DELTA_QUEUE_ELEMENT* dq_element = (DELTA_QUEUE_ELEMENT*)kalloc(1);
	dq_element->ticks_to_wakeup = ticks_to_sleep;
	dq_element->task_state = g_current_task_state;
	dq_element->next = NULL;

	// Insert it into the delta queue
	DELTA_QUEUE_ELEMENT* dq_walk = g_delta_queue->first;
	DELTA_QUEUE_ELEMENT* dq_insert_after = NULL;
	while (dq_walk != NULL && dq_element->ticks_to_wakeup >= dq_walk->ticks_to_wakeup) {
		dq_element->ticks_to_wakeup -= dq_walk->ticks_to_wakeup;
		dq_insert_after = dq_walk;
		dq_walk = dq_walk->next; 
	}

	if (dq_insert_after == NULL) {
		// Insert it at the head position
		if (dq_walk != NULL) g_delta_queue->first->ticks_to_wakeup -= dq_element->ticks_to_wakeup; 
		dq_element->next = g_delta_queue->first;
		g_delta_queue->first = dq_element;
	}
	else {
		if (dq_walk != NULL) dq_insert_after->next->ticks_to_wakeup -= dq_element->ticks_to_wakeup; 
		dq_element->next = dq_insert_after->next;
		dq_insert_after->next = dq_element;
	}

	// print_delta_queue();

	// print_string("TASK STATE to sleep: ");
	// print_dword((uint32_t)g_current_task_state);
	// print_newline();

	serial_write_string("[INFO] Pushing process onto DELTA_QUEUE with ESP 0x");
	serial_write_dword(g_current_task_state->esp);
	serial_write_string(". Process string: ");
	serial_write_string(g_current_task_state->process_string);
	serial_write_string(" for 0x");
	serial_write_dword(ticks_to_sleep);
	serial_write_string(" ticks");
	serial_write_newline();

	// -- TODO -- Make sure there have been no previous calls to disable_interrupts() 

	enable_interrupts();

	// Raise the task switching interrupt
	raise_interrupt_0x80();

	// g_PIT_count_down = ticks_to_sleep;	
	// while (g_PIT_count_down > 0) {
	// 	asm __volatile__ ("hlt");
	// } 

	return;

}

/*
Function: 		time_init
Description: 	Initialize time.h: initialize the PIT; register interrupt handler for the PIT.
				Call after interrupt.h has been initialized.
Return:			NONE
*/
void time_init() {

	disable_interrupts();
	g_delta_queue = (DELTA_QUEUE*)kalloc(1);
	g_delta_queue->first = NULL;
	enable_interrupts();

	PIT_init();
	register_interrupt(interrupt_wrapper_PIT, 0x0);
	PIC_line_enable(0x0);

	register_software_interrupt(interrupt_wrapper_software_blocking, 0x80);

	return;

}

void PIT_init() {

	uint8_t mode_command_PIT = 0b00110110;

	iowriteb(0x43, mode_command_PIT);

	iowriteb(0x40, (uint8_t)PIT_RELOAD_VALUE); // Low
	iowriteb(0x40, (uint8_t)(PIT_RELOAD_VALUE >> 0x8)); // High

	return;

}

void reset_scheduler_time() {

	// print_string("Scheduler time reset");
	// print_newline();

	g_scheduler_time = TIME_SLICE_IN_TICKS;

	return;

}

uint32_t get_total_ticks() {

	return g_total_ticks;
	
}

void interrupt_function_PIT() {

	if (g_PIT_count_down > 0) g_PIT_count_down--;
	if (g_scheduler_time > 0) g_scheduler_time--;
	g_total_ticks++;

	DELTA_QUEUE_ELEMENT* dq_walk = g_delta_queue->first;
	while (dq_walk != NULL && dq_walk->ticks_to_wakeup == 0) dq_walk = dq_walk->next;
	if (dq_walk != NULL) dq_walk->ticks_to_wakeup--;

	iowriteb(0x20, 0x20);

	return;

}

void print_delta_queue() {

	DELTA_QUEUE_ELEMENT* dq_walk = g_delta_queue->first;
	while (dq_walk != NULL) {

		print_string("Delta queue element TASK_STATE: ");
		print_dword((uint32_t)dq_walk->task_state);
		print_string(", ticks_to_wakeup: ");
		print_dword((uint32_t)dq_walk->ticks_to_wakeup);
		print_newline();

		dq_walk = dq_walk->next;

	}

	return;

}