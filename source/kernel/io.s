.section .text
.global ioread
.type ioread, @function
.global ioreadb
.type ioreadb, @function
.global iowrite
.type iowrite, @function
.global iowriteb
.type iowriteb, @function

ioread:

	push %ebp
	mov %esp, %ebp
	push %edx

	mov 0x8(%ebp), %edx
	in (%dx), %eax
	
	pop %edx
	mov %ebp, %esp
	pop %ebp
	ret

ioreadb:

	push %ebp
	mov %esp, %ebp
	push %edx

	mov 0x8(%ebp), %edx
	in (%dx), %al

	pop %edx
	mov %ebp, %esp
	pop %ebp
	ret

ioreadw:

	push %ebp
	mov %esp, %ebp
	push %edx

	mov 0x8(%ebp), %edx
	in (%dx), %ax

	pop %edx
	mov %ebp, %esp
	pop %ebp
	ret

iowrite:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov 0x8(%ebp), %edx
	mov 0xC(%ebp), %eax

	out %eax, (%dx) 

	pop %edx
	pop %eax
	mov %ebp, %esp
	pop %ebp
	ret

iowriteb:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov 0x8(%ebp), %edx
	mov 0xC(%ebp), %eax

	out %al, (%dx)

	pop %edx
	pop %eax
	mov %ebp, %esp
	pop %ebp
	ret


iowritew:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov 0x8(%ebp), %edx
	mov 0xC(%ebp), %eax

	out %ax, (%dx)

	pop %edx
	pop %eax
	mov %ebp, %esp
	pop %ebp
	ret
