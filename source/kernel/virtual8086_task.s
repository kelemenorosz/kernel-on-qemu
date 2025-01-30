.org 0x0000		# code runs at 0x3000:0x0000
.code16			# 16-bit code for virtual 8086
.section .virtual8086, "ax", @progbits

start:

	pop %eax
	pop %ebx
	pop %ecx
	pop %edx
	pop %esi
	pop %edi
	pop %fs
	pop %gs
	pop %es

	int $0x10

	push %es
	push %gs
	push %es
	push %edi	
	push %esi	
	push %edx	
	push %ecx	
	push %ebx	
	push %eax	

	int $0x81
	