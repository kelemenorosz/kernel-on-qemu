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

extern void raise_interrupt_0x80(); // -- Interrupt software blocking
extern void raise_interrupt_0x81(); // -- Interrupt enter VM86 mode
extern void raise_interrupt_0x82(); // -- Interrupt exit VM86 mode

void PIC_line_enable(size_t line);
void PIC_line_disable(size_t line);

#endif /* INTERRUPT_H */