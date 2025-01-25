#ifndef GDT_H
#define GDT_H

#define GDT_ENTRY_TSS 0

void gdt_init();
uint32_t gdt_add(void* tss, uint32_t size, uint32_t type, uint8_t busy);
void lgdt(void* buf, size_t size);

#endif /* GDT_H */