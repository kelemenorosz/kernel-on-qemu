.section .text
.global check_cpuid
.type check_cpuid, @function
.global get_cpuid
.type get_cpuid, @function

check_cpuid:

	push %ebp
	mov %esp, %ebp
	pushfl
	pushfl
	xorl $0x00200000, (%esp) 
	popfl
	pushfl
	pop %eax
	xorl %eax, (%esp)
	popfl
	and $0x00200000, %eax
	cmp $0x0, %eax
	je check_cpuid_false
	check_cpuid_true:
	mov $0x0, %eax
	jmp check_cpuid_exit
	check_cpuid_false:
	mov $0x1, %eax
	check_cpuid_exit:

	mov %ebp, %esp 
	pop %ebp
	ret

get_cpuid:
	
	push %ebp
	mov %esp, %ebp
	push %eax
	push %ebx
	push %ecx
	push %edx
	push %esi

	mov 0x8(%ebp), %eax
	xor %ebx, %ebx
	xor %ecx, %ecx
	xor %edx, %edx
	
	cpuid

	mov 0xC(%ebp), %esi
	mov %eax, (%esi)

	mov 0x10(%ebp), %esi
	mov %ebx, (%esi)

	mov 0x14(%ebp), %esi
	mov %ecx, (%esi)

	mov 0x18(%ebp), %esi
	mov %edx, (%esi)

	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
	pop %eax
	mov %ebp, %esp 
	pop %ebp
	ret
