.section .text
.global interrupt_wrapper_PIT
.type interrupt_wrapper_PIT, @function
.global interrupt_wrapper_software_blocking
.type interrupt_wrapper_software_blocking, @function

interrupt_wrapper_PIT:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_PIT

	cmpl $0x0, g_scheduler_time
	jne interrupt_wrapper_PIT_exit

	cmpl $0x0, g_scheduler_running
	je interrupt_wrapper_PIT_exit

	call scheduler
	call reset_scheduler_time

	push %eax
	push %ebx
	push %ecx
	push %edx
	push %esi
	push %edi
	push %ebp

	mov g_current_task_state, %edi
	mov %esp, (%edi)
	
	mov g_next_task_state, %esi
	mov (%esi), %esp
	
	mov $g_current_task_state, %edi
	mov %esi, (%edi)
	
	pop %ebp
	pop %edi
	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
	pop %eax	

	interrupt_wrapper_PIT_exit:

	mov %ebp, %esp 
	pop %ebp
	iret

interrupt_wrapper_software_blocking:

	push %ebp
	mov %esp, %ebp

	call scheduler

	push %eax
	push %ebx
	push %ecx
	push %edx
	push %esi
	push %edi
	push %ebp

	mov g_current_task_state, %edi
	mov %esp, (%edi)
	
	mov g_next_task_state, %esi
	mov (%esi), %esp
	
	mov $g_current_task_state, %edi
	mov %esi, (%edi)

	pop %ebp
	pop %edi
	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
	pop %eax	

	mov %ebp, %esp 
	pop %ebp
	iret
