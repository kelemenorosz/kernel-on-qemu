#ifndef LIST_H
#define LIST_H

typedef struct __attribute__((__packed__)) LIST_ENTRY {

	void* ptr;
	struct LIST_ENTRY* next;

} LIST_ENTRY;

typedef struct __attribute__((__packed__)) LIST {

	LIST_ENTRY* head;

} LIST;

void list_add(LIST* list, void* buf);

#endif /* LIST_H */