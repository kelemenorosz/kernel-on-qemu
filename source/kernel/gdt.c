#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "gdt.h"
#include "memory.h"
#include "print.h"

typedef struct __attribute__((__packed__)) SEGMENT_DESCRIPTOR {

	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access;
	uint8_t limit1_flags;
	uint8_t base2;

} SEGMENT_DESCRIPTOR;

SEGMENT_DESCRIPTOR* const g_GDTptr = (SEGMENT_DESCRIPTOR*)0x78000; 
uint32_t g_GDT_size = 0x0;

void gdt_init() {

	memset(&g_GDTptr[0], 0, sizeof(SEGMENT_DESCRIPTOR));

	// -- Kernel code segment descriptor

	g_GDTptr[1].limit0 = 0xFFFF;
	g_GDTptr[1].base0 = 0x0;
	g_GDTptr[1].base1 = 0x0;
	g_GDTptr[1].access = 0x9A;
	g_GDTptr[1].limit1_flags = 0xCF;
	g_GDTptr[1].base2 = 0x0;

	// -- Kernel data/stack segment descriptor

	g_GDTptr[2].limit0 = 0xFFFF;
	g_GDTptr[2].base0 = 0x0;
	g_GDTptr[2].base1 = 0x0;
	g_GDTptr[2].access = 0x92;
	g_GDTptr[2].limit1_flags = 0xCF;
	g_GDTptr[2].base2 = 0x0;

	g_GDT_size = 3;

	// -- Task segment descriptor ring 0

	// -- Task segment descriptor ring 3

	lgdt((void*)&g_GDTptr[0], sizeof(SEGMENT_DESCRIPTOR) * g_GDT_size);

	return;

}

uint32_t gdt_add(void* buf, uint32_t size, uint32_t type, uint8_t busy) {

	if (type == GDT_ENTRY_TSS) {

		g_GDT_size++;

		g_GDTptr[g_GDT_size - 1].limit0 = size;
		g_GDTptr[g_GDT_size - 1].base0 = (uint32_t)buf;
		g_GDTptr[g_GDT_size - 1].base1 = (uint32_t)buf >> 0x10;
		if (busy != 0) g_GDTptr[g_GDT_size - 1].access |= 0b1011;
		else g_GDTptr[g_GDT_size - 1].access |= 0b1001;
		g_GDTptr[g_GDT_size - 1].access |= 0b1000 << 0x4;
		g_GDTptr[g_GDT_size - 1].limit1_flags = ((uint32_t)size & 0xF0000) >> 0x10;
		g_GDTptr[g_GDT_size - 1].base2 = (uint32_t)buf >> 0x18;

		lgdt((void*)&g_GDTptr[0], sizeof(SEGMENT_DESCRIPTOR) * g_GDT_size);

		return (g_GDT_size - 1) * sizeof(SEGMENT_DESCRIPTOR);

	}

	return 0;

}

void lgdt(void* buf, size_t size) {

	struct {

		uint16_t offset;
		uint32_t memory_location;
		
	} __attribute__((__packed__)) GDT = {size - 1, (uint32_t)buf};

	asm __volatile__ ("lgdt %0" :: "m"(GDT));

	return;

}
