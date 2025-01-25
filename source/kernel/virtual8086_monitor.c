#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "virtual8086_monitor.h"
#include "print.h"
#include "serial.h"
#include "eflags.h"
#include "exception.h"
#include "io.h"

typedef struct __attribute__((__packed__)) V86_INTERRUPT_STACK_FRAME_ERR {

	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	uint16_t reserved0;
	uint32_t eflags;
	uint32_t esp;
	uint16_t ss;
	uint16_t reserved1;
	uint16_t es;
	uint16_t reserved2;
	uint16_t ds;
	uint16_t reserved3;
	uint16_t fs;
	uint16_t reserved4;
	uint16_t gs;
	uint16_t reserved5;

} V86_INTERRUPT_STACK_FRAME_ERR;

typedef struct __attribute__((__packed__)) V86_TASK_STATE {

	uint32_t eflags;
	uint32_t eip;
	uint16_t cs;
	uint16_t virtual_if;

} V86_TASK_STATE;

typedef struct __attribute__((__packed__)) IVT_ENTRY {

	uint16_t offset;
	uint16_t segment;

} IVT_ENTRY;

V86_TASK_STATE g_v86_task_state;

#define INSTRUCTION_UNKNOWN 0
#define INSTRUCTION_INT 1
#define INSTRUCTION_CLI 2
#define INSTRUCTION_PUSHF 3
#define INSTRUCTION_POPF 4
#define INSTRUCTION_OUT 5

uint32_t virtual8086_monitor(void* stack_frame) {

	V86_INTERRUPT_STACK_FRAME_ERR* v86_sframe = (V86_INTERRUPT_STACK_FRAME_ERR*)stack_frame;
	GP_STACK_FRAME_V86* gp_sframe = (GP_STACK_FRAME_V86*)(stack_frame - sizeof(GP_STACK_FRAME_V86));

	// print_string("CS: ");
	// print_word(v86_sframe->cs);
	// print_newline();
	// print_string("EIP: ");
	// print_dword(v86_sframe->eip);
	// print_newline();

	uint8_t* opcode_ptr = (uint8_t*)((v86_sframe->cs << 4) + v86_sframe->eip);

	serial_write_string("[V86_MONITOR] Flat address: ");
	serial_write_dword((uint32_t)opcode_ptr);
	serial_write_newline();

	serial_write_string("[V86_MONITOR] Instruction opcode: ");
	serial_write_byte(*opcode_ptr);
	serial_write_newline();

	print_string("Instruction opcode: ");
	print_byte(*opcode_ptr);
	print_newline();

	IVT_ENTRY* ivt_ptr = (IVT_ENTRY*)0;
	uint8_t* ring3_stack_u8 = (uint8_t*)((v86_sframe->ss << 4) + v86_sframe->esp); 

	uint32_t is_opcode32 = 0;

	switch(opcode_ptr[0]) {

		case 0xCD:

			// -- INT

			// print_string("INT ");
			// print_byte(opcode_ptr[1]);
			// print_newline();

			// -- Save EFLAGS, CS and EIP on ring3 stack
			// TODO: take virtual interrupt flag into consideration 

			ring3_stack_u8 -= 2;
			*(uint16_t*)ring3_stack_u8 = v86_sframe->eflags;
			ring3_stack_u8 -= 2;
			*(uint16_t*)ring3_stack_u8 = v86_sframe->cs;
			ring3_stack_u8 -= 4;
			*(uint32_t*)ring3_stack_u8 = v86_sframe->eip;

			v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
			v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;

			// -- Save EIP and CS

			g_v86_task_state.eip = v86_sframe->eip;
			g_v86_task_state.cs = v86_sframe->cs;

			// -- Change EIP and CS to point to 8086 interrupt handler

			// print_string("IVT location: ");
			// print_dword(opcode_ptr[1] * 4);
			// print_newline();

			// print_string("Interrupt offset: ");
			// print_word(ivt_ptr[opcode_ptr[1]].offset);
			// print_newline();
			
			// print_string("Interrupt segment: ");
			// print_word(ivt_ptr[opcode_ptr[1]].segment);
			// print_newline();

			v86_sframe->eip = ivt_ptr[opcode_ptr[1]].offset;
			v86_sframe->cs = ivt_ptr[opcode_ptr[1]].segment;

			return INSTRUCTION_INT;

			break;

		case 0xFA:

			// -- CLI

			// -- Clear the virtual interrupt flag

			g_v86_task_state.virtual_if = 0;

			// -- Incement EIP

			v86_sframe->eip++;

			return INSTRUCTION_CLI;
			break;

		case 0x9C:

			// -- PUSHF

			if (is_opcode32) {

			}
			else {

				// -- Decrement ring3 stack

				ring3_stack_u8 -= 2;

				// -- Set EFLAGS

				if (g_v86_task_state.virtual_if) {
					*(uint16_t*)ring3_stack_u8 = v86_sframe->eflags | (1 << EFLAGS_IF_SHIFT);
				}
				else {
					*(uint16_t*)ring3_stack_u8 = v86_sframe->eflags & !(1 << EFLAGS_IF_SHIFT);
				}

				// -- Update ring0 stack frame

				v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
				v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;

			}

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_PUSHF;
			break;

		case 0x9D:

			// -- POPF

			if (is_opcode32) {

			}
			else {

				// -- Update ring0 stack frame EFLAGS 

				v86_sframe->eflags = 0;
				v86_sframe->eflags |= 1 << EFLAGS_IF_SHIFT;
				v86_sframe->eflags |= 1 << EFLAGS_VM_SHIFT;
				v86_sframe->eflags |= *(uint16_t*)ring3_stack_u8;

				// -- Update virtual interrupt flag

				g_v86_task_state.virtual_if = (*(uint16_t*)ring3_stack_u8 & (1 << EFLAGS_IF_SHIFT)) != 0;

				// -- Increment ring3 stack

				ring3_stack_u8 += 2;

				// -- Update ring 0 stack frame ring3 ESP and SS

				v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
				v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;

			}

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_POPF; 
			break;

		case 0xEE:

			// -- OUT DX, AL

			serial_write_string("EAX: ");
			serial_write_dword(gp_sframe->eax0);
			serial_write_newline();

			serial_write_string("EDX: ");
			serial_write_dword(gp_sframe->edx);
			serial_write_newline();

			iowriteb(gp_sframe->edx & 0xFFFF, gp_sframe->eax0 & 0xFF);

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_OUT;
			break;

		default:
			break;

	}

	return 0;

}