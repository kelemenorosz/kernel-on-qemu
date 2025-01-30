#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "virtual8086.h"
#include "memory.h"
#include "interrupt.h"
#include "gdt.h"
#include "print.h"
#include "eflags.h"

uint32_t g_kernel_stack;
uint32_t g_kernel_stack_segment;

typedef struct __attribute__((__packed__)) TASK_STATE_SEGMENT {

	uint16_t link;
	uint16_t reserved0;
	uint32_t esp0;
	uint16_t ss0;
	uint16_t reserved1;
	uint32_t esp1;
	uint16_t ss1;
	uint16_t reserved2;
	uint32_t esp2;
	uint16_t ss2;
	uint16_t reserved3;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;
	uint16_t reserved4;
	uint16_t cs;
	uint16_t reserved5;
	uint16_t ss;
	uint16_t reserved6;
	uint16_t ds;
	uint16_t reserved7;
	uint16_t fs;
	uint16_t reserved8;
	uint16_t gs;
	uint16_t reserved9;
	uint16_t ldt;
	uint16_t reserved10;
	uint16_t reserved_t;
	uint16_t iopb;
	uint32_t ssp;

} TASK_STATE_SEGMENT;

// Careful.
// Any changes made to the size of V86_START_STACK has to be reflected in virtual8086.s:enter_virtual8086.
// As enter_virtual8086 is an interrupt handler, passing arguments is not possible.
// Thus the size change of V86_START_STACK has to be hard coded into the interrupt handler.
// 0xFFFE - sizeof(V86_START_STACK) 
typedef struct __attribute__((__packed__)) V86_START_STACK { 

	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint16_t fs;
	uint16_t gs;
	uint16_t es;

} V86_START_STACK;

void virtual8086_init() {

	register_software_interrupt(enter_virtual8086, 0x81);

	// TODO: relocate code that sets up the TSS

	disable_interrupts();
	TASK_STATE_SEGMENT* tss_ring3 = (TASK_STATE_SEGMENT*)kalloc(1);
	void* ring3_ss0_stack = kalloc(10);		
	enable_interrupts();

	ring3_ss0_stack += 0x10000;

	memset(tss_ring3, 0, sizeof(TASK_STATE_SEGMENT));

	tss_ring3->ss0 = 0x10;
	tss_ring3->esp0 = (uint32_t)ring3_ss0_stack;

	uint32_t tss_ring3_offset = gdt_add(tss_ring3, sizeof(TASK_STATE_SEGMENT), GDT_ENTRY_TSS, 0);

	load_task_register(tss_ring3_offset);


}

void virtual8086(V86_DESC* desc) {

	// -- Set up virtual8086 stack

	void* v86_stack = (void*)0x4FFFE; // 0x4FFFE is stack top
	V86_START_STACK* v86_start_stack_frame = (V86_START_STACK*)(v86_stack - sizeof(V86_START_STACK));

	memset(v86_start_stack_frame, 0, sizeof(V86_START_STACK));

	v86_start_stack_frame->eax = desc->eax;
	v86_start_stack_frame->ebx = desc->ebx;
	v86_start_stack_frame->ecx = desc->ecx;
	v86_start_stack_frame->edx = desc->edx;
	v86_start_stack_frame->esi = desc->esi;
	v86_start_stack_frame->edi = desc->edi;
	v86_start_stack_frame->fs = desc->fs;
	v86_start_stack_frame->gs = desc->gs;
	v86_start_stack_frame->es = desc->es;

	// -- Call virtual8086
	//
	// TODO: Remove PIT interrupt guards
	// 		 For now, no PIT interrupt is received

	PIC_line_disable(0x0);
	raise_interrupt_0x81();
	PIC_line_enable(0x0);

	// -- Return v86 registers

	desc->eax = v86_start_stack_frame->eax;
	desc->ebx = v86_start_stack_frame->ebx;
	desc->ecx = v86_start_stack_frame->ecx;
	desc->edx = v86_start_stack_frame->edx;
	desc->esi = v86_start_stack_frame->esi;
	desc->edi = v86_start_stack_frame->edi;
	desc->fs = v86_start_stack_frame->fs;
	desc->gs = v86_start_stack_frame->gs;
	desc->es = v86_start_stack_frame->es;
	
	return;

}

// void virtual8086_init() {

// 	disable_interrupts();
// 	TASK_STATE_SEGMENT* tss_ring0 = (TASK_STATE_SEGMENT*)kalloc(1);
// 	TASK_STATE_SEGMENT* tss_ring3 = (TASK_STATE_SEGMENT*)kalloc(1);
// 	void* ring3_ss0_stack = kalloc(1);
// 	void* ring3_ss1_stack = kalloc(1);
// 	void* ring3_ss2_stack = kalloc(1);
// 	void* ring0_ss0_stack = kalloc(1);
// 	void* ring0_ss1_stack = kalloc(1);
// 	void* ring0_ss2_stack = kalloc(1);
// 	enable_interrupts();

// 	ring0_ss0_stack += 0x1000;
// 	ring0_ss1_stack += 0x1000;
// 	ring0_ss2_stack += 0x1000;

// 	memset(tss_ring0, 0, sizeof(TASK_STATE_SEGMENT));
// 	memset(tss_ring3, 0, sizeof(TASK_STATE_SEGMENT));

// 	// -- Ring 3 task state segment

// 	tss_ring3->eflags |= 1 << EFLAGS_RESERVED0_SHIFT;
// 	// tss_ring3->eflags |= 1 << EFLAGS_NT_SHIFT;
// 	tss_ring3->eflags |= 1 << EFLAGS_VM_SHIFT;
// 	tss_ring3->eflags |= 1 << EFLAGS_IF_SHIFT;
// 	// tss_ring3->eflags |= 3 << EFLAGS_IOPL_SHIFT;
// 	tss_ring3->esp = 0xfffe;
// 	tss_ring3->cs = 0x3000;
// 	// tss_ring3->eip = (uint32_t)print_byte;
// 	tss_ring3->eip = 0x0;
// 	tss_ring3->ss = 0x4000;
// 	tss_ring3->ds = 0x3000;
// 	tss_ring3->fs = 0x3000;
// 	tss_ring3->gs = 0x3000;
// 	tss_ring3->es = 0x3000;
// 	tss_ring3->ss0 = 0x10;
// 	tss_ring3->ss1 = 0x10;
// 	tss_ring3->ss2 = 0x10;
// 	tss_ring3->esp0 = (uint32_t)ring3_ss0_stack;
// 	tss_ring3->esp1 = (uint32_t)ring3_ss1_stack;
// 	tss_ring3->esp2 = (uint32_t)ring3_ss2_stack;

// 	// -- Set TSS_RING0 and TSS_RING3 in the GDT

// 	uint32_t tss_ring0_offset = gdt_add(tss_ring0, sizeof(TASK_STATE_SEGMENT), GDT_ENTRY_TSS, 0);
// 	uint32_t tss_ring3_offset = gdt_add(tss_ring3, sizeof(TASK_STATE_SEGMENT), GDT_ENTRY_TSS, 1);

// 	// -- Ring 0 task state segment

// 	tss_ring0->ss0 = 0x10;
// 	tss_ring0->ss1 = 0x10;
// 	tss_ring0->ss2 = 0x10;
// 	tss_ring0->esp0 = (uint32_t)ring0_ss0_stack;
// 	tss_ring0->esp1 = (uint32_t)ring0_ss1_stack;
// 	tss_ring0->esp2 = (uint32_t)ring0_ss2_stack;
// 	tss_ring0->link = tss_ring3_offset; 

// 	// -- Set TSS_RING0 as current TSS

// 	load_task_register(tss_ring0_offset);

// 	call_virtual8086();

// 	return;

// }