#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "io.h"
#include "pci.h"

uint8_t g_enum_bus = 0x0;
uint8_t g_enum_device = 0x0;
uint8_t g_enum_function = 0x0;

//===========================================================================================================================================================
// Non-exported function declarations

uint32_t 	read_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void 		write_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data);


//===========================================================================================================================================================
// Exported functions

void set_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data) {

	write_config_data(bus, device, function, offset, write_data);
	return;

}

void set_PCI_offset_token(PCI_ENUM_TOKEN* token, uint8_t offset, uint32_t write_data) {

	write_config_data(token->bus, token->device, token->function, offset, write_data);
	return;

}

uint32_t get_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {

	return (uint32_t)read_config_data(bus, device, function, offset);

}

uint32_t get_PCI_offset_token(PCI_ENUM_TOKEN* token, uint8_t offset) {

	return (uint32_t)read_config_data(token->bus, token->device, token->function, offset);

}

void find_PCI_device_no_progif(PCI_ENUM_TOKEN* token, PCI_DEVICE_CLASS* device_class) {

	for (size_t i = 0; i < 0x100; ++i) {
	 	for (size_t j = 0; j < 0x20; ++j) {

	 		uint16_t vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x0);
	 		if (vendorID != 0xFFFF) {

				uint32_t deviceClass = get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x8);
				if ((uint8_t)(deviceClass >> 0x18) == device_class->classCode && (uint8_t)(deviceClass >> 0x10) == device_class->subClass) {
					token->bus 		= (uint8_t)i;
					token->device 	= (uint8_t)j;
					token->function = 0x0;
				}					
	 			
	 			uint8_t header_type = (uint8_t)(get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0xC) >> 0x10);
	 			if (header_type & 0x80) {

	 				for (size_t k = 1; k < 0x8; ++k) {
	 				
	 					vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x0);
	 					if (vendorID != 0xFFFF) {

							uint32_t deviceClass = get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x8);
							if ((uint8_t)(deviceClass >> 0x18) == device_class->classCode && (uint8_t)(deviceClass >> 0x10) == device_class->subClass) {
								token->bus 		= (uint8_t)i;
								token->device 	= (uint8_t)j;
								token->function = (uint8_t)k;
							}					

	 					}

	 				}
	 			
	 			}

	 		}

	 	}
	}

	return;

}

void find_PCI_device(PCI_ENUM_TOKEN* token, PCI_DEVICE_CLASS* device_class) {

	for (size_t i = 0; i < 0x100; ++i) {
	 	for (size_t j = 0; j < 0x20; ++j) {

	 		uint16_t vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x0);
	 		if (vendorID != 0xFFFF) {

				uint32_t deviceClass = get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x8);
				if ((uint8_t)(deviceClass >> 0x18) == device_class->classCode && (uint8_t)(deviceClass >> 0x10) == device_class->subClass && (uint8_t)(deviceClass >> 0x8) == device_class->progIF) {
					token->bus 		= (uint8_t)i;
					token->device 	= (uint8_t)j;
					token->function = 0x0;
				}					
	 			
	 			uint8_t header_type = (uint8_t)(get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0xC) >> 0x10);
	 			if (header_type & 0x80) {

	 				for (size_t k = 1; k < 0x8; ++k) {
	 				
	 					vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x0);
	 					if (vendorID != 0xFFFF) {

							uint32_t deviceClass = get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x8);
							if ((uint8_t)(deviceClass >> 0x18) == device_class->classCode && (uint8_t)(deviceClass >> 0x10) == device_class->subClass && (uint8_t)(deviceClass >> 0x8) == device_class->progIF) {
								token->bus 		= (uint8_t)i;
								token->device 	= (uint8_t)j;
								token->function = (uint8_t)k;
							}					

	 					}

	 				}
	 			
	 			}

	 		}

	 	}
	}

	return;

}

uint32_t enum_PCI_count() {

	uint32_t count = 0x0;

	for (size_t i = 0; i < 0x100; ++i) {
	 	for (size_t j = 0; j < 0x20; ++j) {

	 		uint16_t vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x0);
	 		if (vendorID != 0xFFFF) {

	 			count++;

	 			uint8_t header_type = (uint8_t)(get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0xC) >> 0x10);
	 			if (header_type & 0x80) {

	 				for (size_t k = 1; k < 0x8; ++k) {
	 				
	 					vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x0);
	 					if (vendorID != 0xFFFF) {

	 						count++;

	 					}

	 				}
	 			
	 			}

	 		}

	 	}
	}

	return count;

}

void enum_PCI(void* buf) {

	PCI_ENUM_TOKEN* token = (PCI_ENUM_TOKEN*)buf;

	for (size_t i = 0; i < 0x100; ++i) {
	 	for (size_t j = 0; j < 0x20; ++j) {

	 		uint16_t vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0x0);
	 		if (vendorID != 0xFFFF) {

	 			token->bus = (uint8_t)i;
	 			token->device = (uint8_t)j;
	 			token->function = 0;
	 			token++;

	 			uint8_t header_type = (uint8_t)(get_PCI_offset((uint8_t)i, (uint8_t)j, 0x0, 0xC) >> 0x10);
	 			if (header_type & 0x80) {

	 				for (size_t k = 1; k < 0x8; ++k) {
	 				
	 					vendorID = (uint16_t)get_PCI_offset((uint8_t)i, (uint8_t)j, (uint8_t)k, 0x0);
	 					if (vendorID != 0xFFFF) {
				 						
				 			token->bus = (uint8_t)i;
				 			token->device = (uint8_t)j;
				 			token->function = (uint8_t)k;
				 			token++;

	 					}

	 				}
	 			
	 			}

	 		}

	 	}
	}

	return;

}

//===========================================================================================================================================================
// Non-exported function definitions

uint32_t read_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {

	uint32_t bus_32b = (uint32_t) bus;
	uint32_t device_32b = (uint32_t) device;
	uint32_t function_32b = (uint32_t) function;
	uint32_t offset_32b = (uint32_t) offset;

	uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus_32b << 16) | (device_32b << 11) | (function_32b << 8) | offset_32b);

	iowrite(0xCF8, config_address);
	uint32_t return_value = ioread(0xCFC);

	return return_value;

}

void write_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data) {

	uint32_t bus_32b = (uint32_t) bus;
	uint32_t device_32b = (uint32_t) device;
	uint32_t function_32b = (uint32_t) function;
	uint32_t offset_32b = (uint32_t) offset;

	uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus_32b << 16) | (device_32b << 11) | (function_32b << 8) | offset_32b);

	iowrite(0xCF8, config_address);
	iowrite(0xCFC, write_data);

	return;

}
