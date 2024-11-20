#ifndef INTERRUPT_H
#define INTERRUPT_H

#define PIC1_COMMAND 	0x20
#define PIC2_COMMAND 	0xA0
#define PIC1_DATA 		0x21
#define PIC2_DATA 		0xA1

void interrupt_init();
void register_interrupt(void(*interrupt_function)(void), size_t line);
void register_software_interrupt(void(*interrupt_function)(void), size_t line);

void enable_interrupts();
void disable_interrupts();

extern void raise_interrupt_0x80();

void PIC_line_enable(size_t line);

#endif /* INTERRUPT_H */