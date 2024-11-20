.section .text
.global get_eflags
.type get_eflags, @function

get_eflags:

	push %ebp
	mov %esp, %ebp
	pushfl
	
	mov (%esp), %eax 

	popfl
	mov %ebp, %esp
	pop %ebp
	ret
