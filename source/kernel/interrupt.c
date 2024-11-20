#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "io.h"

typedef struct __attribute__((__packed__)) IDT_ENTRY {

	uint16_t offset_1;
	uint16_t segment_selector;
	uint8_t reserved;
	uint8_t type;
	uint16_t offset_2;

} IDT_ENTRY; 

IDT_ENTRY* const g_IDTPtr = (IDT_ENTRY*)0x70000; 

void load_IDTR();
void PIC_init();
extern void interrupt_wrapper_spurious();
void interrupt_function_spurious();

/*
Function: 		interrupt_init
Description: 	Initializes interrupt.h: Sets IDTR; Masks off every PIT interrupt; Enables interrupts; Registers spurious interrupt handler. 
Return:			NONE
*/
void interrupt_init() {

	load_IDTR();
	PIC_init();
	enable_interrupts();
	register_interrupt(interrupt_wrapper_spurious, 0x7);

	return;

}

/*
Function: 		PIC_line_enable
Description: 	Enables 'line' interrupt on the PICs. 
Return:			NONE
*/
void PIC_line_enable(size_t line) {

	uint8_t data_port = 0x0;
	uint8_t line_shift = 0x0;
	if (line > 0x8) {
		data_port = PIC2_DATA;
		line_shift = line - 0x8;
	}
	else {
		data_port = PIC1_DATA;
		line_shift = line;
	}

	uint8_t pic_mask = ioreadb(data_port);
	pic_mask = pic_mask & ~(1 << line_shift);

	iowriteb(data_port, pic_mask);

	return;

}

/*
Function: 		interrupt_function_spurious
Description: 	Spurious interrupt handler.
				Does nothing.
Return:			NONE
*/
void interrupt_function_spurious() {

	return;

}

/*
Function: 		PIC_init
Description: 	Initializes the two PICs.
				All interrupts will be masked off. 
Return:			NONE
*/
void PIC_init() {

	iowriteb(0x20, 0x11);
	iowriteb(0xA0, 0x11);

	iowriteb(0x21, 0x20);
	iowriteb(0xA1, 0x28);

	iowriteb(0x21, 0x04);
	iowriteb(0xA1, 0x02);

	iowriteb(0x21, 0x01);
	iowriteb(0xA1, 0x01);

	iowriteb(0x21, 0xFB);
	iowriteb(0xA1, 0xFF);

	return;

}

/*
Function: 		load_IDTR
Description: 	Load the IDTR register.
Return:			NONE
*/
void load_IDTR() {

	struct {

		uint16_t offset;
		uint32_t memory_location;
		
	} __attribute__((__packed__)) LIDT = {0x7FF, (uint32_t)g_IDTPtr};

	asm __volatile__ ("lidt %0" :: "m"(LIDT));

	return;

}


/*
Function: 		register_interrupt
Description: 	Registers interrupt handler 'interrupt_function' in the IDT on entry 'line' + 32.
				Previously registered interrupt handler becomes lost.
				Registers interrupt gates: (0x8E).
				Disables and enables interrupts inside the function.
				Call after interrupt.h has been initialized.
Return:			NONE
*/
void register_interrupt(void(*interrupt_function)(void), size_t line) {

	disable_interrupts();

	IDT_ENTRY* idt_entry = g_IDTPtr + 0x20 + line;
	uint32_t interrupt_offset = (uint32_t)interrupt_function;

	idt_entry->offset_1 			= (uint16_t)(interrupt_offset >> 0x00);
	idt_entry->offset_2 			= (uint16_t)(interrupt_offset >> 0x10);
	idt_entry->segment_selector 	= 0x08;
	idt_entry->reserved 			= 0x00;
	idt_entry->type 				= 0x8E;

	enable_interrupts();

	return;

}

/*
Function: 		register_software_interrupt
Description: 	Registers interrupt handler 'interrupt_function' in the IDT on entry 'line'.
				Previously registered interrupt handler becomes lost.
				Registers interrupt gates: (0x8E).
				Disables and enables interrupts inside the function.
				Call after interrupt.h has been initialized.
Return:			NONE
*/
void register_software_interrupt(void(*interrupt_function)(void), size_t line) {

	disable_interrupts();

	IDT_ENTRY* idt_entry = g_IDTPtr + line;
	uint32_t interrupt_offset = (uint32_t)interrupt_function;

	idt_entry->offset_1 			= (uint16_t)(interrupt_offset >> 0x00);
	idt_entry->offset_2 			= (uint16_t)(interrupt_offset >> 0x10);
	idt_entry->segment_selector 	= 0x08;
	idt_entry->reserved 			= 0x00;
	idt_entry->type 				= 0x8E;

	enable_interrupts();
	
	return;

}

/*
Function: 		enable_interrupts
Description: 	Enables interrupts.
				Instruction wrapper for STI.
Return:			NONE
*/
void __attribute__((optimize("O0"))) enable_interrupts() {

	asm __volatile__ ("sti");

	return;

}

/*
Function: 		disable_interrupts
Description: 	Disables interrupts.
				Instruction wrapper for CLI.
Return:			NONE
*/
void __attribute__((optimize("O0"))) disable_interrupts() {

	asm __volatile__ ("cli");

	return;

}

