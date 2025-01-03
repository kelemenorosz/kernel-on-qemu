#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "list.h"
#include "interrupt.h"
#include "memory.h"

void list_add(LIST* list, void* buf) {

	disable_interrupts();
	LIST_ENTRY* entry = (LIST_ENTRY*)kalloc(1);
	enable_interrupts();
 	
 	entry->ptr = buf;
 	entry->next = list->head;
 	list->head = entry;

	return;

}
