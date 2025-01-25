#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "exception.h"
#include "interrupt.h"
#include "serial.h"

void interrupt_function_exception_gp();
void interrupt_function_exception_ts();
void interrupt_function_exception_ud(void* stack_frame);
void interrupt_function_exceptions(void* stack_frame);

extern void interrupt_wrapper_exception_gp();
extern void interrupt_wrapper_exception_ts();
extern void interrupt_wrapper_exception_ud();
extern void interrupt_wrapper_exceptions();

/*
Function: 		exception_init
Description: 	Initializes exception.h: Sets up exceptions handlers.
				Call after interrupt.h and serial.h have been initialized. 
Return:			NONE
*/
void exception_init() {

	for (uint8_t i = 0; i < 0x20; ++i) {
		if (i == 6) {
			register_software_interrupt(interrupt_wrapper_exception_ud, i);
		}
		else if (i == 10) {
			register_software_interrupt(interrupt_wrapper_exception_ts, i);
		}
		else if (i == 13) {
			register_software_interrupt(interrupt_wrapper_exception_gp, i);
		}
		else {
			register_software_interrupt(interrupt_wrapper_exceptions, i);
		}
	}

	register_software_interrupt(interrupt_wrapper_exceptions, 0x50);

	return;

}

/*
Function: 		interrupt_function_exception_gp
Description: 	Interrupt handler for General Protection fault. 
Return:			NONE
*/
void interrupt_function_exception_gp() {

	serial_write_string("[FATAL_ERROR] General Protection fault.");
	serial_write_newline();

	return;

}

/*
Function: 		interrupt_function_exception_ts
Description: 	Interrupt handler for Invalid TSS fault. 
Return:			NONE
*/
void interrupt_function_exception_ts() {

	serial_write_string("[FATAL_ERROR] Invalid TSS fault.");
	serial_write_newline();

	return;

}

/*
Function: 		interrupt_function_exceptions
Description: 	Generic interrupt handler for exceptions. 
Return:			NONE
*/
void interrupt_function_exceptions(void* stack_frame) {

	serial_write_string("[FATAL_ERROR] Exception raised.");
	serial_write_newline();
	serial_write_string("[FATAL_ERROR] Stack at 0x");
	serial_write_dword((uint32_t)stack_frame);
	serial_write_newline();

}

/*
Function: 		interrupt_function_exception_ud
Description: 	Interrupt handler for Undefined Opcode fault. 
Return:			NONE
*/
void interrupt_function_exception_ud(void* stack_frame) {

	serial_write_string("[FATAL_ERROR] Undefined Opcode.");
	serial_write_newline();
	serial_write_string("EIP: 0x");
	serial_write_dword(*(uint32_t*)stack_frame);
	serial_write_string(" CS: 0x");
	serial_write_dword(*(uint32_t*)(stack_frame + 0x4));
	serial_write_string(" EFLAGS: 0x");
	serial_write_dword(*(uint32_t*)(stack_frame + 0x8));
	serial_write_newline();

}
