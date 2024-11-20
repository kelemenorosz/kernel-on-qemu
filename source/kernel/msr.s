.section .text
.global read_msr
.type read_msr, @function

read_msr:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %ecx
	push %edx
	
	mov 0x8(%ebp), %ecx
	rdmsr

	mov 0xC(%ebp), %ecx
	mov %eax, (%ecx)

	mov 0x10(%ebp), %ecx
	mov %edx, (%ecx)

	pop %edx
	pop %ecx
	pop %eax
	mov %ebp, %esp 
	pop %ebp
	ret
	