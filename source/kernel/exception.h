#ifndef EXCEPTION_H
#define EXCEPTION_H

typedef struct __attribute__((__packed__)) GP_STACK_FRAME_V86 { 

	uint32_t eax2;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax1;
	uint32_t eax0;
	uint32_t ebp;

} GP_STACK_FRAME_V86;

void exception_init();

#endif /* EXCEPTION_H */