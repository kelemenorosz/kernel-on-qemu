.section .text
.global _start
.type _start, @function

_start:

	jmp $0x08,$far_start

far_start:
	
	mov $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	mov $0x90000, %eax
	mov %eax, %esp 

	call kernel_main
	
hang:
	
	hlt
	jmp hang
