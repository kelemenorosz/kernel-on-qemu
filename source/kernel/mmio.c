#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t mmio_read(uint32_t addr, uint32_t offset) {

	return *(volatile uint32_t*)(addr + offset);

}

void mmio_write(uint32_t addr, uint32_t offset, uint32_t value) {

	*(volatile uint32_t*)(addr + offset) = value;

	return;

}