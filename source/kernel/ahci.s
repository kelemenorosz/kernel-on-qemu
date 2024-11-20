.section .text
.global interrupt_wrapper_ahci
.type interrupt_wrapper_ahci, @function

interrupt_wrapper_ahci:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_ahci

	mov %ebp, %esp 
	pop %ebp
	iret
