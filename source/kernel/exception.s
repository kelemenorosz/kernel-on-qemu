.section .text
.global interrupt_wrapper_exception_ts
.type interrupt_wrapper_exception_ts, @function
.global interrupt_wrapper_exception_gp
.type interrupt_wrapper_exception_gp, @function
.global interrupt_wrapper_exception_ud
.type interrupt_wrapper_exception_ud, @function
.global interrupt_wrapper_exceptions
.type interrupt_wrapper_exceptions, @function

interrupt_wrapper_exception_gp:

	push %ebp
	mov %esp, %ebp
	
	push %eax

	# Check if task is calling from VM86

	mov 0x14(%esp), %eax
	and $0x20000, %eax
	cmp $0x0, %eax
	je interrupt_wrapper_exception_gp_not_v86 	# If calling from VM86, DS is not set.
												# Be careful when jumping before re-setting DS.
												# Fine here. Only jumps when not calling from VM86.
interrupt_wrapper_exception_gp_v86:
	
	# -- If task is VM86

	mov %ss, %ax # Set DS to SS.
	mov %ax, %ds # mov %ss, %ds doesn't work

	push %eax
	push %ecx
	push %edx

	mov %esp, %eax
	add $0x14, %eax 
	push %eax 	# Set stack pointer at label beginning as parameter
				# %eax = %esp + 'nr. of "push" instructions since interrupt start' * 4

	call virtual8086_monitor # Call VM86 monitor
	add $0x4, %esp

	// cmp $0x01, %eax # Check if return code is INSTRUCTION_INT
	// je interrupt_wrapper_exception_gp_v86_int

	pop %edx
	pop %ecx
	pop %eax

// interrupt_wrapper_exception_gp_v86_int:

// 	pop %eax
// 	pop %ecx
// 	pop %edx

// 	# -- If GP raised due to INT opcode PUSH EFLAGS(low 16-bits); CS and EIP

// 	pop %eax  		# Restore stack to initial conditions
// 	mov %ebp, %esp  #
// 	pop %ebp		#

// 	sub $0xC, %esp	# Make space for EFLAGS; CS and EIP
// 	push %eax       # PUSH EAX (1)

// 	pushfl				# PUSH EFLAGS(low 16-bits)
// 	pop %eax 			#
// 	and $0xFFFF, %eax 	#
// 	push %eax 			#

// 	push %cs 			# PUSH CS
// 	push %eip 			# PUSH EIP

// 	pop %eax 			# POP EAX (1)
// 	iret

	jmp interrupt_wrapper_exception_gp_exit

interrupt_wrapper_exception_gp_not_v86:

	# If task is not VM86

	call interrupt_function_exception_gp

interrupt_wrapper_exception_gp_exit:

	pop %eax
	
	mov %ebp, %esp
	pop %ebp

	add $0x4, %esp

	iret	

interrupt_wrapper_exception_ts:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_exception_ts

	mov %ebp, %esp
	pop %ebp

	iret	

interrupt_wrapper_exception_ud:

	push %ebp
	mov %esp, %ebp
	
	push %eax

	mov %esp, %eax
	add $0x8, %eax
	push %eax

	call interrupt_function_exception_ud
	add $0x4, %esp

	pop %eax

	mov %ebp, %esp
	pop %ebp

	iret	

interrupt_wrapper_exceptions:

	push %ebp
	mov %esp, %ebp

	push %eax

	mov %esp, %eax
	add $0x8, %eax
	push %eax

	call interrupt_function_exceptions
	add $0x4, %esp

	pop %eax

	mov %ebp, %esp
	pop %ebp

	iret
