#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "scheduler.h"
#include "print.h"
#include "interrupt.h"
#include "time.h"
#include "serial.h"

typedef	struct __attribute__((__packed__)) PROCESS_FRAME {

	uint32_t ebp1;
	uint32_t edi;
	uint32_t esi;
	uint32_t edx; 
	uint32_t ecx; 
	uint32_t ebx; 
	uint32_t eax; 
	uint32_t ebp0;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;

} PROCESS_FRAME;

typedef struct __attribute__((__packed__)) TASK_QUEUE_ELEMENT {

	TASK_STATE* task_state;
	struct TASK_QUEUE_ELEMENT* next;

} TASK_QUEUE_ELEMENT;

typedef struct __attribute__((__packed__)) TASK_QUEUE {

	TASK_QUEUE_ELEMENT* first;
	TASK_QUEUE_ELEMENT* last;

} TASK_QUEUE;

TASK_STATE* g_task_state_list = NULL;

TASK_STATE* g_current_task_state = 0x0;
TASK_STATE* g_next_task_state = 0x0;

TASK_QUEUE* g_task_queue = NULL;

TASK_STATE* g_idle_task_state = NULL;

uint32_t g_scheduler_running = 0x0;

extern DELTA_QUEUE* g_delta_queue;

TASK_STATE* pop_task_queue();
void push_task_queue(TASK_STATE* task_state);

void idle_task();
void register_idle_task(void(*task_func)(void));

/* 
Function: 		scheduler_init
Description: 	Initializes scheduler.h: allocates memory for initial process task state information.
				Sets as first element in 'g_task_state_list'.
Return:			NONE
*/
void scheduler_init() {

	disable_interrupts();

	TASK_STATE* task_state = (TASK_STATE*)kalloc(1);
	task_state->next = NULL;
	task_state->is_blocking = 0x0;

	g_current_task_state = task_state;
	g_task_state_list = task_state;

	g_task_queue = (TASK_QUEUE*)kalloc(1);
	push_task_queue(task_state);

	// Create idle task process
	register_idle_task(idle_task);

	enable_interrupts();

	return;

}


/* 
Function: 		register_idle_task
Description: 	Same as task_create, with the exception that it does not get added to the process queue.
Return:			NONE
*/
void register_idle_task(void(*task_func)(void)) {

	disable_interrupts();
	TASK_STATE* task_state = (TASK_STATE*)kalloc(1);
	void* stack_bottom = kalloc(0x10); 
	void* stack_top = stack_bottom + 0x10 * 0x1000; 
	enable_interrupts();

	PROCESS_FRAME* process_frame = stack_top - sizeof(PROCESS_FRAME);
	process_frame->eflags = 0x202; // This should be set with a bit more thought. For now bits 9 and 1 are set.
	process_frame->cs = 0x08;
	process_frame->eip = (uint32_t)task_func;
	process_frame->ebp0 = 0x0; // This can be 0 - could be something else
	process_frame->eax = 0x0;
	process_frame->ebx = 0x0;
	process_frame->ecx = 0x0;
	process_frame->edx = 0x0;
	process_frame->esi = 0x0;
	process_frame->edi = 0x0;
	process_frame->ebp1 = (uint32_t)&process_frame->ebp0; // This has to be the address of ebp1. Will be moved into esp, prior to popping ebp0 off the stack

	task_state->esp = (uint32_t)process_frame;
	task_state->is_blocking = 0x0;

	g_idle_task_state = task_state; 

	return;

}

/* 
Function: 		task_create
Description: 	Creates a task state information structure for a new task.
				Allocates stack space of 16 pages.
				Adds task state information to 'g_task_state_list'.
Return:			NONE
*/
void task_create(void(*task_func)(void)){

	disable_interrupts();
	TASK_STATE* task_state = (TASK_STATE*)kalloc(1);
	void* stack_bottom = kalloc(0x10); 
	void* stack_top = stack_bottom + 0x10 * 0x1000; 
	enable_interrupts();

	PROCESS_FRAME* process_frame = stack_top - sizeof(PROCESS_FRAME);
	process_frame->eflags = 0x202; // This should be set with a bit more thought. For now bits 9 and 1 are set.
	process_frame->cs = 0x08;
	process_frame->eip = (uint32_t)task_func;
	process_frame->ebp0 = 0x0; // This can be 0 - could be something else
	process_frame->eax = 0x0;
	process_frame->ebx = 0x0;
	process_frame->ecx = 0x0;
	process_frame->edx = 0x0;
	process_frame->esi = 0x0;
	process_frame->edi = 0x0;
	process_frame->ebp1 = (uint32_t)&process_frame->ebp0; // This has to be the address of ebp1. Will be moved into esp, prior to popping ebp0 off the stack

	task_state->esp = (uint32_t)process_frame;
	task_state->is_blocking = 0x0;

	task_state->next = g_task_state_list;
	g_task_state_list = task_state;

	disable_interrupts();
	push_task_queue(task_state);
	enable_interrupts();

	return;

}

/* 
Function: 		pop_task_queue
Description: 	Pops an element off the task queue.
Return:			Pointer to the popped off TASK_STATE structure 
*/
TASK_STATE* pop_task_queue() {

	if (g_task_queue->first == NULL) {

		return NULL;
	
	} else if (g_task_queue->first == g_task_queue->last) {

		TASK_QUEUE_ELEMENT* tq_element = g_task_queue->first;
		g_task_queue->first = NULL;
		g_task_queue->last = NULL;
		TASK_STATE* task_state = tq_element->task_state;

		serial_write_string("[INFO] Popping process from TASK_QUEUE with ESP 0x");
		serial_write_dword(task_state->esp);
		serial_write_newline();

		kfree(tq_element, 1);
		return task_state;
	
	} else {

		TASK_QUEUE_ELEMENT* tq_element = g_task_queue->first;
		g_task_queue->first = g_task_queue->first->next;
		TASK_STATE* task_state = tq_element->task_state;

		serial_write_string("[INFO] Popping process from TASK_QUEUE with ESP 0x");
		serial_write_dword(task_state->esp);
		serial_write_newline();

		kfree(tq_element, 1);
		return task_state;

	}

}

/* 
Function: 		push_task_queue
Description: 	Pushes an element onto the task queue.
Return:			NONE
*/
void push_task_queue(TASK_STATE* task_state) {

	serial_write_string("[INFO] Pushing process onto TASK_QUEUE with ESP 0x");
	serial_write_dword(task_state->esp);
	serial_write_newline();

	TASK_QUEUE_ELEMENT* tq_element = kalloc(1);
	tq_element->task_state = task_state;
	tq_element->next = NULL;

	if (g_task_queue->last == NULL) {

		g_task_queue->first = tq_element;
		g_task_queue->last = tq_element;
	
	} else {

		g_task_queue->last->next = tq_element;
		g_task_queue->last = tq_element;

	}

	return;

}

/* 
Function: 		scheduler
Description: 	Called in interrupt_wrapper_PIT.
				It should only be called once scheduler_start has been called.
Return:			NONE
*/
void scheduler() {

	// print_string("Current task state: ");
	// print_dword((uint32_t)g_current_task_state);
	// print_newline();

	// if (g_current_task_state == g_task_state_list) {
	// 	g_next_task_state = g_task_state_list->next;
	// }
	// else {
	// 	g_next_task_state = g_task_state_list;
	// }

	// Select next running process
	while (g_task_queue->first != NULL) {

		TASK_STATE* task_state = pop_task_queue();
		if (!task_state->is_blocking) {
			g_next_task_state = task_state;
			push_task_queue(task_state);
			break;
		} 

	}

	if (g_task_queue->first == NULL) {
		// All processes are blocking, call the idle task
		
		// print_string("This should never be printed");
		// print_newline();
		g_next_task_state = g_idle_task_state; 

	}

	// Clear the delta queue as much as you can and add the tasks onto the task queue
	DELTA_QUEUE_ELEMENT* dq_element = g_delta_queue->first;
	while (dq_element != NULL && dq_element->ticks_to_wakeup == 0) {
		serial_write_string("[INFO] Popping of process from DELTA_QUEUE with ESP 0x");
		serial_write_dword(dq_element->task_state->esp);
		serial_write_newline();
		dq_element->task_state->is_blocking = 0x0;
		push_task_queue(dq_element->task_state);
		g_delta_queue->first = dq_element->next;
		kfree(dq_element, 1);
		dq_element = g_delta_queue->first;
	}

	return;

}

/* 
Function: 		scheduler_start
Description: 	Called after scheduler.h has been initialized.
				After this function has been called the scheduler will select a next process to run whenever the PIT interrupt handler is raised and when the time of a process has ran out.
Return:			NONE
*/
void scheduler_start() {

	g_scheduler_running = 0x1;

	return;

}

void print_task_state_list() {

	TASK_STATE* task_state = g_task_state_list;
	while (task_state != NULL) {

		print_string("Task state this: ");
		print_dword((uint32_t)task_state);
		print_newline();

		print_string("Task state esp: ");
		print_dword(task_state->esp);
		print_newline();

		print_string("Task state next: ");
		print_dword((uint32_t)task_state->next);
		print_newline();

		task_state = task_state->next;

	}

	return;

}

void idle_task() {

	// disable_interrupts();

	// print_string("Entering idle task:");
	// print_newline();

	// asm __volatile__ ("hlt");

	while (true) {

		asm __volatile__ ("hlt");

	}

	return;

}


