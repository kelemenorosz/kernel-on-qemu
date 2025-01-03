#ifndef MMIO_H
#define MMIO_H

uint32_t mmio_read(uint32_t addr, uint32_t offset);
void mmio_write(uint32_t addr, uint32_t offset, uint32_t value);

#endif /* MMIO_H */