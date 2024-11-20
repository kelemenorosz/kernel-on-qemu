#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "io.h"

void interrupt_function_keyboard();
extern void interrupt_wrapper_keyboard();

/*
Function: 		keyboard_init
Description: 	Initializes keyboard.h: Registers keyboard interrupt handler.
				Call after interrupt.h has been initialized.
Return:			NONE
*/
void keyboard_init() {

	register_interrupt(interrupt_wrapper_keyboard, 0x1);
	PIC_line_enable(0x1);

	return;

}

/*
Function: 		interrupt_function_keyboard
Description: 	Interrupt handler for the keyboard. 
Return:			NONE
*/
void interrupt_function_keyboard() {

	uint16_t* vga_buffer = (uint16_t*)0xB8000;

 	uint8_t keyboard_input = ioreadb(0x60);
 	keyboard_input += 1;

 	*(vga_buffer + 0x4F) += 1;
	
 	iowriteb(0x20, 0x20);

	return;

}
