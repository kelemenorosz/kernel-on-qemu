#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "list.h"
#include "socket.h"

typedef struct __attribute__((__packed__)) TASK_STATE {

	uint32_t esp;
	uint32_t is_blocking;
	uint32_t on_task_queue;
	char* process_string;
	LIST* sockets;
	SOCKET* arp_sck;
	struct TASK_STATE* next;

} TASK_STATE;

typedef uint32_t PPARAM;

void scheduler_init();
void task_create(void(*task_func)(void), char* process_string);
void task_create_param(void(*task_func)(PPARAM), char* process_string, uint32_t param);

void scheduler();
void scheduler_start();

void print_task_state_list();

extern TASK_STATE* g_current_task_state;
extern LIST* g_processes;

#endif /* SCHEDULER_H */