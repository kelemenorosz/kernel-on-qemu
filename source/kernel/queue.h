#ifndef QUEUE_H
#define QUEUE_H

typedef struct __attribute__((__packed__)) QUEUE_ENTRY {

	void* ptr;
	struct QUEUE_ENTRY* next;

} QUEUE_ENTRY;

typedef struct __attribute__((__packed__)) QUEUE {

	QUEUE_ENTRY* head;
	QUEUE_ENTRY* tail;

} QUEUE;

void* queue_pop(QUEUE* queue);
void queue_push(QUEUE* queue, void* ptr);

#endif /* QUEUE_H */