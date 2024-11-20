#ifndef PCI_H
#define PCI_H

typedef struct __attribute__((__packed__)) PCI_ENUM_TOKEN {

	uint8_t bus;
	uint8_t device;
	uint8_t function;

} PCI_ENUM_TOKEN;

typedef struct __attribute__((__packed__)) PCI_DEVICE_CLASS {

	uint8_t classCode;
	uint8_t subClass;
	uint8_t progIF;

} PCI_DEVICE_CLASS;

void 		set_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data);
void 		set_PCI_offset_token(PCI_ENUM_TOKEN* token, uint8_t offset, uint32_t write_data);
uint32_t 	get_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint32_t	get_PCI_offset_token(PCI_ENUM_TOKEN* token, uint8_t offset);

void find_PCI_device_no_progif(PCI_ENUM_TOKEN* token, PCI_DEVICE_CLASS* device_class);
void find_PCI_device(PCI_ENUM_TOKEN* token, PCI_DEVICE_CLASS* device_class);

uint32_t 	enum_PCI_count();
void 		enum_PCI(void* buf);

#endif /* PCI_H */