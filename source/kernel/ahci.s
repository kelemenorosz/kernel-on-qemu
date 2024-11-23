.section .text
.global interrupt_wrapper_ahci
.type interrupt_wrapper_ahci, @function

interrupt_wrapper_ahci:

	push %ebp
	mov %esp, %ebp

	push %eax
	push %ecx
	push %edx

	call interrupt_function_ahci

	pop %edx
	pop %ecx
	pop %eax

	mov %ebp, %esp 
	pop %ebp
	iret
