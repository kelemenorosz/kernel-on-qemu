.section .text
.global enter_virtual8086
.type enter_virtual8086, @function
.global exit_virtual8086
.type exit_virtual8086, @function
.global load_task_register
.type load_task_register, @function

load_task_register:

	push %ebp
	mov %esp, %ebp
	push %edx

	mov 0x8(%ebp), %edx
	ltr %dx

	pop %edx
	mov %ebp, %esp
	pop %ebp
	ret


enter_virtual8086:

	// -- Save ring0 register states

	push %ebp
	push %eax
	push %ebx
	push %ecx
	push %edx
	push %edi
	push %esi
	push %ds
	push %fs
	push %gs
	push %es

	// -- Save ring0 SS:ESP 
	
	mov %esp, (g_kernel_stack)
	mov %ss, (g_kernel_stack_segment)

	// -- Load VM86 interrupt stack frame 

	sub $0x4, %esp
	movl $0x3000, (%esp) // GS
	sub $0x4, %esp
	movl $0x3000, (%esp) // FS
	sub $0x4, %esp
	movl $0x3000, (%esp) // DS
	sub $0x4, %esp
	movl $0x3000, (%esp) // ES
	sub $0x4, %esp
	movl $0x4000, (%esp) // SS
	sub $0x4, %esp
	movl $0xFFE0, (%esp) // ESP
	sub $0x4, %esp
	movl $0x20202, (%esp) // EFLAGS (VM, IF)
	sub $0x4, %esp
	movl $0x3000, (%esp) // CS
	sub $0x4, %esp
	movl $0x0000, (%esp) // EIP

	iret

exit_virtual8086:

	// To get here:
	//	 
	// - VM86 calls IRET 0x81.	 
	// - GP handler calls exit_virtual8086().

	// -- Load ring0 SS:ESP

	mov (g_kernel_stack), %esp
	mov (g_kernel_stack_segment), %ss

	// -- Restore register states

	pop %es
	pop %gs
	pop %fs
	pop %ds
	pop %esi
	pop %edi
	pop %edx
	pop %ecx
	pop %ebx
	pop %eax
	pop %ebp

	iret


// call_virtual8086:
	
// 	push %ebp
// 	mov %esp, %ebp

// 	# Set NT flag

// 	pushfl

// 	push %eax

// 	mov 0x4(%esp), %eax
// 	or $0x4000, %eax
// 	mov %eax, 0x4(%esp)

// 	mov %cr4, %eax
// 	or $0x200, %eax
// 	mov %eax, %cr4

// 	pop %eax
// 	popfl
// 	pushfl

// 	# Push CS

// 	# push %cs
// 	push 0x2

// 	lea print_byte, %eax
// 	# push %eax
// 	push $0x5000

// 	iret
