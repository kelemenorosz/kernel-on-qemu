.section .text
.global call_virtual8086
.type call_virtual8086, @function
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

call_virtual8086:
	
	push %ebp
	mov %esp, %ebp

	# Set NT flag

	pushfl

	push %eax

	mov 0x4(%esp), %eax
	or $0x4000, %eax
	mov %eax, 0x4(%esp)

	mov %cr4, %eax
	or $0x200, %eax
	mov %eax, %cr4

	pop %eax
	popfl
	pushfl

	# Push CS

	# push %cs
	push 0x2

	lea print_byte, %eax
	# push %eax
	push $0x5000

	iret
