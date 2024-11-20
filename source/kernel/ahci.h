#ifndef AHCI_H
#define AHCI_H

#include "pci.h"

void AHCI_init(PCI_ENUM_TOKEN* pci_token);

void AHCI_write(uint32_t port, void* buf, uint32_t lba_low, uint32_t lba_high, uint32_t sector_count);
void AHCI_read(uint32_t port, void* buf, uint32_t lba_low, uint32_t lba_high, uint32_t sector_count);

void AHCI_print_devices();

#endif /* AHCI_H */