.section .text
.global interrupt_wrapper_keyboard
.type interrupt_wrapper_keyboard, @function

interrupt_wrapper_keyboard:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_keyboard

	mov %ebp, %esp 
	pop %ebp
	iret
