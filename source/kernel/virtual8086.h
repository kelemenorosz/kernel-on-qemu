#ifndef VIRTUAL8086_H
#define VIRTUAL8086_H

void virtual8086_init();
extern void load_task_register(uint32_t offset);
extern void call_virtual8086();

#endif /* VIRTUAL8086 */