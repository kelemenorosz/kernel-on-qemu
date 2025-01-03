#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "queue.h"
#include "memory.h"
#include "interrupt.h"

void* queue_pop(QUEUE* queue) {

	QUEUE_ENTRY* queue_entry = queue->head;
	if (queue->head != queue->tail) {
		queue->head = queue_entry->next;
	}
	else {
		queue->head = NULL;
		queue->tail = NULL;
	}

	void* ptr = queue_entry->ptr;
	disable_interrupts();
	kfree(queue_entry, 1);
	enable_interrupts();

	return ptr;

}

void queue_push(QUEUE* queue, void* ptr) {

	disable_interrupts();
	QUEUE_ENTRY* queue_entry = (QUEUE_ENTRY*)kalloc(1);
	enable_interrupts();
	queue_entry->ptr = ptr;
	queue_entry->next = NULL;

	if (queue->head != NULL && queue->tail != NULL) {
		queue->tail->next = queue_entry;
		queue->tail = queue_entry;
	}
	else {
		queue->head = queue_entry;
		queue->tail = queue_entry;
	}

	return;

}