#ifndef VIRTUAL8086_H
#define VIRTUAL8086_H

typedef struct __attribute__((__packed__)) V86_DESC { 

	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;
	uint16_t gs;
	uint16_t fs;

} V86_DESC;

void virtual8086_init();
void virtual8086(V86_DESC* desc);
extern void load_task_register(uint32_t offset);
extern void enter_virtual8086();
extern void exit_virtual8086();

#endif /* VIRTUAL8086 */