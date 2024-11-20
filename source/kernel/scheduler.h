#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct __attribute__((__packed__)) TASK_STATE {

	uint32_t esp;
	uint32_t is_blocking;
	struct TASK_STATE* next;

} TASK_STATE;

void scheduler_init();
void task_create(void(*task_func)(void));

void scheduler();
void scheduler_start();

void print_task_state_list();

#endif /* SCHEDULER_H */