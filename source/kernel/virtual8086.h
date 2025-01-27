#ifndef VIRTUAL8086_H
#define VIRTUAL8086_H

void virtual8086_init();
void virtual8086();
extern void load_task_register(uint32_t offset);
extern void enter_virtual8086();
extern void exit_virtual8086();

#endif /* VIRTUAL8086 */