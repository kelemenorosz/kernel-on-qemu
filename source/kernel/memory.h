#ifndef MEMORY_H
#define MEMORY_H

void memset(void* ptr, uint8_t value, size_t num);
void memcpy(void* destination, const void* source, size_t num);

void memory_init();
void print_memmap();

// void* alloc_page(size_t count);

void* kalloc(size_t page_count);
void kfree(void* address, size_t page_count);

void print_freelists();

#endif /* MEMORY_H */