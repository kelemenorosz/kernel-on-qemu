#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "virtual8086_monitor.h"
#include "virtual8086.h"
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
#define INSTRUCTION_IN 6
#define INSTRUCTION_IRET 7

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

	if (v86_sframe->eip == 0xFFFF) {
		serial_write_string("[UHOH]");
		serial_write_newline();
		return INSTRUCTION_UNKNOWN;
	}

	switch(opcode_ptr[0]) {

		case 0xCD:

			// -- INT

			if (opcode_ptr[1] == 0x81) {
				exit_virtual8086();

				// exit_virtual8086() contains a stack switch to the proper ring0 stack
				// code execution doesn't return to the GP handler
			
			}

			// -- Save EFLAGS, CS and EIP on ring3 stack
			// TODO: take virtual interrupt flag into consideration 
			ring3_stack_u8 -= 2;
			*(uint16_t*)ring3_stack_u8 = v86_sframe->eflags | (1 << EFLAGS_IF_SHIFT);
			ring3_stack_u8 -= 2;
			*(uint16_t*)ring3_stack_u8 = v86_sframe->cs;
			ring3_stack_u8 -= 2;
			*(uint16_t*)ring3_stack_u8 = (uint16_t)(v86_sframe->eip & 0xFFFF);

			v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
			v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;

			// -- Save EIP and CS

			g_v86_task_state.eip = v86_sframe->eip;
			g_v86_task_state.cs = v86_sframe->cs;

			// -- Change EIP and CS to point to 8086 interrupt handler

			v86_sframe->eip = ivt_ptr[opcode_ptr[1]].offset;
			v86_sframe->cs = ivt_ptr[opcode_ptr[1]].segment;

			return INSTRUCTION_INT;
			break;

		case 0xCF:

			// -- IRET

			// -- Pop off EFLAGS, CS and EIP from ring3 stack

			v86_sframe->eip = *(uint16_t*)ring3_stack_u8;
			ring3_stack_u8 += 2;
			v86_sframe->cs = *(uint16_t*)ring3_stack_u8;
			ring3_stack_u8 += 2;
			v86_sframe->eflags = (v86_sframe->eflags & 0xFFFF0000) + *(uint16_t*)ring3_stack_u8;
			ring3_stack_u8 += 2;

			// -- Re-set ring3 stack

			v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
			v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;

			// -- Increment EIP

			v86_sframe->eip += 2;

			return INSTRUCTION_IRET;
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
					*(uint16_t*)ring3_stack_u8 = (v86_sframe->eflags & (0xDFF)) | (1 << EFLAGS_IF_SHIFT);
				}
				else {
					*(uint16_t*)ring3_stack_u8 = (v86_sframe->eflags & (0xDFF)) & !(1 << EFLAGS_IF_SHIFT);
				}

				// -- Update ring0 stack frame

				// Don't ever do this again.
				// Flattening the SS:ESP and splitting it into segment:offset again is a NO-NO.
				// Doing it this way messed up whatever the BIOS interrupt code was up to.	
				//
				// v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
				// v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;
				//
				// TODO: Check what PUSHF would do in case of v86_sframe->esp overflow. I would think nothing. It'd wrap around.

				v86_sframe->esp -= 2;

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

				// v86_sframe->esp = (uint32_t)ring3_stack_u8 & 0xFFFF;
				// v86_sframe->ss = ((uint32_t)ring3_stack_u8 & 0xF0000) >> 4;
				v86_sframe->esp += 2;

			}

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_POPF; 
			break;

		case 0xEE:

			// -- OUT DX, AL

			serial_write_string("EAX: 0x");
			serial_write_dword(gp_sframe->eax0);
			serial_write_newline();

			serial_write_string("EDX: 0x");
			serial_write_dword(gp_sframe->edx);
			serial_write_newline();

			iowriteb(gp_sframe->edx & 0xFFFF, gp_sframe->eax0 & 0xFF);

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_OUT;
			break;

		case 0xEF:

			// -- OUT DX, AX
			// -- OUT DX, EAX

			serial_write_string("EAX: 0x");
			serial_write_dword(gp_sframe->eax0);
			serial_write_newline();

			serial_write_string("EDX: 0x");
			serial_write_dword(gp_sframe->edx);
			serial_write_newline();

			if (is_opcode32) {

				// -- OUT 32 bits

			}
			else {

				// -- OUT 16 bits

				iowritew(gp_sframe->edx & 0xFFFF, gp_sframe->eax0 & 0xFFFF);

			}

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_OUT;
			break;

		case 0xEC:

			// -- IN AL, DX

			serial_write_string("EDX: 0x");
			serial_write_dword(gp_sframe->edx);
			serial_write_newline();

			// TODO: I don't know if this would preserve the upper 24-bits
			gp_sframe->eax0 = (gp_sframe->eax0 & 0xFFFFFF00) + ioreadb(gp_sframe->edx & 0xFFFF);

			serial_write_string("EAX: 0x");
			serial_write_dword(gp_sframe->eax0);
			serial_write_newline();

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_IN;
			break;

		case 0xED:

			// -- IN AX, AX
			// -- IN EAX, AX

			serial_write_string("EDX: 0x");
			serial_write_dword(gp_sframe->edx);
			serial_write_newline();

			if (is_opcode32) {

				// -- IN 32 bits

			}
			else {

				// -- IN 16 bits

				gp_sframe->eax0 = (gp_sframe->eax0 & 0xFFFF0000) + ioreadw(gp_sframe->edx & 0xFFFF);

			}

			serial_write_string("EAX: 0x");
			serial_write_dword(gp_sframe->eax0);
			serial_write_newline();

			// -- Increment EIP

			v86_sframe->eip++;

			return INSTRUCTION_IN;
			break;

		default:
			break;

	}

	return 0;

}