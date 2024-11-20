.section .text
.global interrupt_wrapper_ethernet
.type interrupt_wrapper_ethernet, @function

interrupt_wrapper_ethernet:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_ethernet

	mov %ebp, %esp 
	pop %ebp
	iret
