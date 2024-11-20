.section .text
.global interrupt_wrapper_spurious
.type interrupt_wrapper_spurious, @function
.global raise_interrupt_0x80
.type raise_interrupt_0x80, @function

interrupt_wrapper_spurious:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_spurious

	mov %ebp, %esp 
	pop %ebp
	iret

raise_interrupt_0x80:
	
	push %ebp
	mov %esp, %ebp

	int $0x80

	mov %ebp, %esp 
	pop %ebp
	ret
