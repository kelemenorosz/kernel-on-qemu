#ifndef TIME_H
#define TIME_H

#include "scheduler.h"

typedef struct __attribute__((__packed__)) DELTA_QUEUE_ELEMENT {

	TASK_STATE* task_state;
	uint32_t ticks_to_wakeup;
	struct DELTA_QUEUE_ELEMENT* next; 

} DELTA_QUEUE_ELEMENT;

typedef struct __attribute__((__packed__)) DELTA_QUEUE {

	DELTA_QUEUE_ELEMENT* first;

} DELTA_QUEUE;

void time_init();

void sleep(uint32_t ticks_to_sleep);
uint32_t get_total_ticks();

#endif /* TIME_H */ 