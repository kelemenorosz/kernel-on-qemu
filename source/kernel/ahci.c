#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "io.h"
#include "ahci.h"
#include "print.h"
#include "pci.h"
#include "time.h"
#include "memory.h"

typedef struct __attribute__((__packed__)) AHCI_GENERIC_HOST_CONTROL {

	volatile uint32_t CAP;
	volatile uint32_t GHC;
	volatile uint32_t IS;
	volatile uint32_t PI;
	volatile uint32_t VS;
	volatile uint32_t CCC_CTL;
	volatile uint32_t CCC_PORTS;
	volatile uint32_t EM_LOC;
	volatile uint32_t EM_CLT;
	volatile uint32_t CAP2;
	volatile uint32_t BOHC;

} AHCI_GENERIC_HOST_CONTROL;

#define AHCI_GHC_AE_SHIFT		0x31
#define AHCI_GHC_MRSM_SHIFT		0x02
#define AHCI_GHC_IE_SHIFT		0x01
#define AHCI_GHC_HR_SHIFT		0x00

#define AHCI_HBA_CAP_NP_SHIFT 	0x00

typedef struct __attribute__((__packed__)) AHCI_PORT_SPECIFIC_REGISTERS {

	volatile uint32_t PxCLB;
	volatile uint32_t PxCLBU;
	volatile uint32_t PxFB;
	volatile uint32_t PxFBU;
	volatile uint32_t PxIS;
	volatile uint32_t PxIE;
	volatile uint32_t PxCMD;
	volatile uint32_t Reserved;
	volatile uint32_t PxTFD;
	volatile uint32_t PxSIG;
	volatile uint32_t PxSSTS;
	volatile uint32_t PxSCTL;
	volatile uint32_t PxSERR;
	volatile uint32_t PxSACT;
	volatile uint32_t PxCI;
	volatile uint32_t PxSNTF;
	volatile uint32_t PxFBS;
	volatile uint32_t PxDEVSLP;

} AHCI_PORT_SPECIFIC_REGISTERS;

#define AHCI_PxIE_CPDE_SHIFT	0x1F
#define AHCI_PxIE_TFEE_SHIFT	0x1E
#define AHCI_PxIE_HBFE_SHIFT	0x1D
#define AHCI_PxIE_HBDE_SHIFT	0x1C
#define AHCI_PxIE_IFE_SHIFT		0x1B
#define AHCI_PxIE_INFE_SHIFT	0x1A
#define AHCI_PxIE_OFE_SHIFT		0x18
#define AHCI_PxIE_IPME_SHIFT	0x17
#define AHCI_PxIE_PRCE_SHIFT	0x16
#define AHCI_PxIE_DMPE_SHIFT	0x07
#define AHCI_PxIE_PCE_SHIFT	 	0x06
#define AHCI_PxIE_DPE_SHIFT		0x05
#define AHCI_PxIE_UFE_SHIFT		0x04
#define AHCI_PxIE_SDBE_SHIFT	0x03
#define AHCI_PxIE_DSE_SHIFT		0x02
#define AHCI_PxIE_PSE_SHIFT		0x01
#define AHCI_PxIE_DHRE_SHIFT	0x00

#define AHCI_PxCMD_ICC_SHIFT	0x1C
#define AHCI_PxCMD_ASP_SHIFT	0x1B
#define AHCI_PxCMD_ALPE_SHIFT	0x1A
#define AHCI_PxCMD_DLAE_SHIFT	0x19
#define AHCI_PxCMD_ATAPI_SHIFT	0x18
#define AHCI_PxCMD_APSTE_SHIFT	0x17
#define AHCI_PxCMD_FBSCP_SHIFT	0x16
#define AHCI_PxCMD_ESP_SHIFT	0x15
#define AHCI_PxCMD_CPD_SHIFT	0x14
#define AHCI_PxCMD_MPSP_SHIFT	0x13
#define AHCI_PxCMD_HPCP_SHIFT	0x12
#define AHCI_PxCMD_PMA_SHIFT	0x11
#define AHCI_PxCMD_CPS_SHIFT	0x10
#define AHCI_PxCMD_CR_SHIFT		0x0F
#define AHCI_PxCMD_FR_SHIFT		0x0E
#define AHCI_PxCMD_MPSS_SHIFT	0x0D
#define AHCI_PxCMD_CCS_SHIFT	0x08
#define AHCI_PxCMD_FRE_SHIFT	0x04
#define AHCI_PxCMD_CLO_SHIFT	0x03
#define AHCI_PxCMD_POD_SHIFT	0x02
#define AHCI_PxCMD_SUD_SHIFT	0x01
#define AHCI_PxCMD_ST_SHIFT		0x00

#define AHCI_PxSSTS_DET_SHIFT	0x00

typedef struct __attribute__((__packed__)) AHCI_COMMAND_HEADER {

	volatile uint32_t DESC_INFO;
	volatile uint32_t PRDBC;
	volatile uint32_t CTBA;	
	volatile uint32_t CTBAU;
	volatile uint32_t Reserved[4];

} AHCI_COMMAND_HEADER;

#define AHCI_COMMAND_HEADER_DESC_INFO_PRDTL_SHIFT	0x10
#define AHCI_COMMAND_HEADER_DESC_INFO_PMP_SHIFT		0x0C
#define AHCI_COMMAND_HEADER_DESC_INFO_C_SHIFT		0x0A
#define AHCI_COMMAND_HEADER_DESC_INFO_B_SHIFT		0x09
#define AHCI_COMMAND_HEADER_DESC_INFO_R_SHIFT		0x08
#define AHCI_COMMAND_HEADER_DESC_INFO_P_SHIFT		0x07
#define AHCI_COMMAND_HEADER_DESC_INFO_W_SHIFT		0x06
#define AHCI_COMMAND_HEADER_DESC_INFO_A_SHIFT		0x05
#define AHCI_COMMAND_HEADER_DESC_INFO_CFL_SHIFT		0x00

typedef struct __attribute__((__packed__)) SATA_REG_H2D_FIS {

	volatile uint8_t FISType;
	volatile uint8_t Misc;
	volatile uint8_t Command;
	volatile uint8_t Features;
	volatile uint8_t LBA0;
	volatile uint8_t LBA1;
	volatile uint8_t LBA2;
	volatile uint8_t Device;
	volatile uint8_t LBA3;
	volatile uint8_t LBA4;
	volatile uint8_t LBA5;
	volatile uint8_t FeaturesExp;
	volatile uint8_t SectorCount;
	volatile uint8_t SectorCountExp;
	volatile uint8_t Reserved_1;
	volatile uint8_t Control;
	volatile uint8_t Reserved_2[4];

} SATA_REG_H2D_FIS;

#define FIS_TYPE_REG_H2D 0x27

#define ATA_COMMAND_READ_DMA_EXT 	0x25
#define ATA_COMMAND_WRITE_DMA_EXT 	0x35

typedef struct __attribute__((__packed__)) AHCI_PRDT_ENTRY {

	uint32_t DBA;
	uint32_t DBAU;
	uint32_t Reserved;
	uint32_t DBC;

} AHCI_PRDT_ENTRY;

typedef struct __attribute__((__packed__)) AHCI_COMMAND_TABLE {

	volatile uint8_t CFIS[64];
	volatile uint8_t ACMD[16];
	volatile uint8_t Reserved[48];
	volatile uint8_t PRDT[1];

} AHCI_COMMAND_TABLE;

typedef struct __attribute__((__packed__)) AHCI_DEVICE_LIST {

	struct AHCI_DEVICE_LIST* next;
	uint32_t lba_size;
	uint32_t lba_count_low;
	uint32_t lba_count_high;
	uint32_t port;
	uint8_t	model_number[41];

} AHCI_DEVICE_LIST;

extern const size_t VGA_WIDTH;
extern uint16_t* const g_VGABuffer;

AHCI_GENERIC_HOST_CONTROL* g_ahci_ghc = 0x0;
AHCI_DEVICE_LIST* g_ahci_device_list = NULL;

extern void interrupt_wrapper_ahci();
void interrupt_function_ahci();

void AHCI_execute_command(AHCI_COMMAND_TABLE* command_table, uint16_t prdtl, uint8_t port);

void identify_swap(void* buf, uint32_t len);

void register_device(void* model_number_buf, uint32_t port, uint32_t lba_size, uint32_t lba_count_low, uint32_t lba_count_high);

/*
Function: 		AHCI_init
Description: 	Initializes ahci.h: starts the AHCI controller; sets up the interrupt handler; checks for connected devices. 
Return:			NONE
*/
void AHCI_init(PCI_ENUM_TOKEN* pci_token) {

	uint32_t abar = get_PCI_offset(pci_token->bus, pci_token->device, pci_token->function, 0x24); 
	uint32_t mem_register_location = abar & 0xFFFFFFF0;

	g_ahci_ghc = (AHCI_GENERIC_HOST_CONTROL*)mem_register_location;

	if (g_ahci_ghc->CAP2 & 0x1) {
		print_string("AHCI BIOS Handoff needed.");
		print_newline();
	}
	else {

		// Reset the controller - GHC.HR = 1
		g_ahci_ghc->GHC |= 1 << AHCI_GHC_HR_SHIFT;
		while (g_ahci_ghc->GHC & (1 << AHCI_GHC_HR_SHIFT)) sleep(1);

		// Enable global interrupts - GHC.IE = 1
		g_ahci_ghc->GHC |= 1 << AHCI_GHC_IE_SHIFT;

		// Set up interrupt handler
		uint8_t interrupt_line = (uint8_t)get_PCI_offset(pci_token->bus, pci_token->device, pci_token->function, 0x3C); 
		register_interrupt(interrupt_wrapper_ahci, interrupt_line);
		PIC_line_enable(interrupt_line);

		uint32_t status_command = (uint32_t)get_PCI_offset(pci_token->bus, pci_token->device, pci_token->function, 0x04);

		print_string("AHCI PCI status and command: ");
		print_dword(status_command);
		print_newline();

		// Check for implemented ports and devices present
		for (uint8_t i = 0; i < 32; ++i) {

			// Check if port is implemented - PI.i bit is set
			if (g_ahci_ghc->PI & (1 << i)) {

				AHCI_PORT_SPECIFIC_REGISTERS* port_spec_regs = (void*)g_ahci_ghc + 0x100 + i*0x80;				
				
				// Check if device is present
				if (port_spec_regs->PxSSTS & (0x3 << AHCI_PxSSTS_DET_SHIFT)) {

					// Create command list; Set PxCLB and PxCLBU
					AHCI_COMMAND_HEADER* command_list = (AHCI_COMMAND_HEADER*)kalloc(1);
					port_spec_regs->PxCLB = (uint32_t)command_list;

					// Create received FIS structure; Set PxFB and PxFBU
					volatile void* received_FIS_buf = kalloc(1);
					port_spec_regs->PxFB = (uint32_t)received_FIS_buf;

					// Enable interrupts; Set PxIE.DHRE = 1
					port_spec_regs->PxIE |= 1 << AHCI_PxIE_DHRE_SHIFT; 

					// Start FIS recieve; Set PxCMD.FRE = 1
					port_spec_regs->PxCMD |= 1 << AHCI_PxCMD_FRE_SHIFT;
					while (!(port_spec_regs->PxCMD & (1 << AHCI_PxCMD_FR_SHIFT))) sleep(1);

					// Start port; Set PxCMD.ST = 1
					port_spec_regs->PxCMD |= 1 << AHCI_PxCMD_ST_SHIFT;

					// IDENTIFY DEVICE

					AHCI_COMMAND_TABLE* identify_cmd_table = (AHCI_COMMAND_TABLE*)kalloc(1);
					memset((void*)identify_cmd_table, 0x0, sizeof(AHCI_COMMAND_TABLE));
					SATA_REG_H2D_FIS* cfis = (SATA_REG_H2D_FIS*)(&identify_cmd_table->CFIS);
					AHCI_PRDT_ENTRY* prdt = (AHCI_PRDT_ENTRY*)(&identify_cmd_table->PRDT);
					memset((void*)prdt, 0x0, sizeof(AHCI_PRDT_ENTRY));
					volatile uint8_t* identify_buf = (uint8_t*)kalloc(1); 

					cfis->FISType 	= FIS_TYPE_REG_H2D;
					cfis->Misc 		= 0x80;
					cfis->Command 	= 0xEC;

					prdt->DBA 	= (uint32_t)identify_buf;
					prdt->DBC	= 0x1FF;
					prdt->DBC	|= (1 << 0x1F);

					AHCI_execute_command(identify_cmd_table, 0x1, i);

					identify_swap((void*)identify_buf + 10 * 2, 20);
					identify_swap((void*)identify_buf + 23 * 2, 8);
					identify_swap((void*)identify_buf + 27 * 2, 40);

					uint16_t* identify_buf_u16 = (uint16_t*)identify_buf;
					uint32_t* identify_buf_u32 = (uint32_t*)identify_buf;

					uint32_t lba_count_low	 	= 0x0;
					uint32_t lba_count_high		= 0x0;

					uint32_t lba_size			= 0x0;

					// Check for 48-bit address feature set support
					uint8_t addr_feature_48_support = 0x0;
					if (identify_buf_u16[83] & (1 << 10)) addr_feature_48_support = 0x1;

					// Get logical sector count
					if (addr_feature_48_support) {
						lba_count_low 	= identify_buf_u32[50]; 
						lba_count_high	= identify_buf_u32[51]; 
					}

					// Check for large logical sector size
					uint8_t large_sector_size_support = 0x0;
					if (!(identify_buf_u16[106] & (1 << 15)) && (identify_buf_u16[106] & (1 << 14)) && (identify_buf_u16[106] & (1 << 12))) large_sector_size_support = 0x1; 

					// Get logical sector size
					if (large_sector_size_support) {
						lba_size = identify_buf_u16[117] + (identify_buf_u16[118] << 0x10); 
					}
					else {
						lba_size = 0x200;
					}

					// Register device

					if (addr_feature_48_support) register_device((void*)identify_buf + 27 * 2, i, lba_size, lba_count_low, lba_count_high);

				}

			}

		}

	}
	
	return;

}

/*
Function: 		AHCI_print_devices
Description: 	Prints out all available devices registered to the global 'g_ahci_ahci_device_list'.
Return:			NONE
*/
void AHCI_print_devices() {

	AHCI_DEVICE_LIST* device = g_ahci_device_list;

	while (device != NULL) {
		print_string("Port ");
		print_byte((uint8_t)device->port);
		print_string(", model nr: ");
		print_string((char*)device->model_number);
		print_newline();
		print_string("sector size: ");
		print_dword(device->lba_size);
		print_string(", sector count: ");
		print_dword(device->lba_count_high);
		print_dword(device->lba_count_low);
		print_newline();

		device = device->next;
	}

}

/*
Function: 		register_device
Description: 	Adds a new entry to the global 'g_ahci_device_list'.
				Contains port number, model number, sector size and sector count for the device. 
Return:			NONE
*/
void register_device(void* model_number_buf, uint32_t port, uint32_t lba_size, uint32_t lba_count_low, uint32_t lba_count_high) {

	AHCI_DEVICE_LIST* device_list_entry = (AHCI_DEVICE_LIST*)kalloc(1);
	
	device_list_entry->port = port;
	device_list_entry->lba_size = lba_size;
	device_list_entry->lba_count_low = lba_count_low;
	device_list_entry->lba_count_high = lba_count_high;
	memcpy((void*)&device_list_entry->model_number[0], model_number_buf, 40);
	device_list_entry->model_number[40] = '\0';
	
	if (g_ahci_device_list == NULL) {
		device_list_entry->next = NULL;
		g_ahci_device_list = device_list_entry;
	}
	else {
		device_list_entry->next = g_ahci_device_list;
		g_ahci_device_list = device_list_entry;
	}

	return;

}

/*
Function: 		identify_swap
Description: 	Swaps pairs of bytes in 'buf' for 'len' bytes.
				Utility function for the IDENTIFY_DEVICE return ASCII strings. 
Return:			NONE
*/
void identify_swap(void* buf, uint32_t len) {

	uint8_t* buf_u8 = (uint8_t*)buf;

	for (uint32_t i = 0; i < len; ++i) {
		if (i % 2 == 0) {
			uint8_t swap = buf_u8[i];
			buf_u8[i] = buf_u8[i + 1];
			buf_u8[i + 1] = swap;
		}
	}

	return;

}

/*
Function: 		AHCI_execute_command
Description: 	Executes 'command_table' with 'prdtl' PRDT entries on the device connected on 'port'. 
				It may enter an infinite loop if something goes wrong.
				Waits until the port specific register PxCI bit corresponding to the command list entry goes to 0. 
Return:			NONE
*/
void AHCI_execute_command(AHCI_COMMAND_TABLE* command_table, uint16_t prdtl, uint8_t port) {

	AHCI_PORT_SPECIFIC_REGISTERS* port_spec_regs = (void*)g_ahci_ghc + 0x100 + port*0x80;			
	AHCI_COMMAND_HEADER* command_list = (AHCI_COMMAND_HEADER*)port_spec_regs->PxCLB;
	uint8_t command_list_index = 0x0;

	// Check for empty command table entry
	for (uint8_t i = 0; i < 32; ++i) {
		if (!((1 << i) & port_spec_regs->PxCI)) {
			command_list_index = i;
			break;
		} 
	}

	command_list[command_list_index].DESC_INFO 	= 0x0;
	command_list[command_list_index].DESC_INFO 	|= (sizeof(SATA_REG_H2D_FIS) / 4) << AHCI_COMMAND_HEADER_DESC_INFO_CFL_SHIFT;
	command_list[command_list_index].DESC_INFO 	|=  prdtl << AHCI_COMMAND_HEADER_DESC_INFO_PRDTL_SHIFT;

	command_list[command_list_index].PRDBC 		= 0x0;

	command_list[command_list_index].CTBA 		= (uint32_t)command_table;
	command_list[command_list_index].CTBAU 		= 0x0;

	// Set PxCI.command_list_index
	port_spec_regs->PxCI |= 1 << command_list_index;
	
	while (port_spec_regs->PxCI & (1 << command_list_index)) sleep(1);

	return;

}


/*
Function: 		AHCI_write
Description: 	Write 'sector_count' sectors starting from 'lba_high' << 0x20 + 'lba_low' to disk connected on 'port'. 
				Doesn't allocate more than 0x1000 bytes for the command table.
				Cannot use all 65535 Physical Region Descriptor Table entries.
				Uses the ATA/ACS WRITE_DMA_EXT 48-bit addressing mode command. 
Return:			NONE
*/
void AHCI_write(uint32_t port, void* buf, uint32_t lba_low, uint32_t lba_high, uint32_t sector_count) {

	AHCI_DEVICE_LIST* device = g_ahci_device_list;
	while (device != NULL) {
		if (device->port == port) {
			break;
		}
		device = device->next;
	}

	AHCI_COMMAND_TABLE* command_table = (AHCI_COMMAND_TABLE*)kalloc(1);
	memset((void*)command_table, 0x0, sizeof(AHCI_COMMAND_TABLE));

	SATA_REG_H2D_FIS* cfis = (SATA_REG_H2D_FIS*)(&command_table->CFIS);	
	
	cfis->FISType 		= FIS_TYPE_REG_H2D;
	cfis->Misc			= 0x80;
	cfis->Command 		= ATA_COMMAND_WRITE_DMA_EXT;

	cfis->LBA0			= (uint8_t)(lba_low >> 0x00);
	cfis->LBA1			= (uint8_t)(lba_low >> 0x08);
	cfis->LBA2			= (uint8_t)(lba_low >> 0x10);
	cfis->LBA3			= (uint8_t)(lba_low >> 0x18);
	cfis->LBA4			= (uint8_t)(lba_high >> 0x0);
	cfis->LBA5			= (uint8_t)(lba_high >> 0x8);

	cfis->SectorCount 		= (uint8_t)(sector_count >> 0x0);
	cfis->SectorCountExp	= (uint8_t)(sector_count >> 0x8);
	cfis->Device			= 0x40;

	AHCI_PRDT_ENTRY* prdt = (AHCI_PRDT_ENTRY*)(&command_table->PRDT);
	uint32_t prdt_entry_count = 0x0;
	
	uint8_t* buf_u8 = (uint8_t*)buf;
	uint8_t* buf_end_u8 = (uint8_t*)(buf + sector_count * device->lba_size);
	uint32_t write_length = 0x0;

	while (buf_u8 < buf_end_u8) {
		if ((buf_end_u8 - buf_u8) > 0x2000) {
			write_length = 0x2000;
		} 
		else {
			write_length = buf_end_u8 - buf_u8;
		}

		memset((void*)prdt, 0x0, sizeof(AHCI_PRDT_ENTRY));

		prdt->DBA	= (uint32_t)buf_u8;
		prdt->DBC 	= write_length - 1;
		prdt->DBC	|= (1 << 0x1F);

		prdt_entry_count++;
		prdt++;
		buf_u8 += write_length;

	}

	AHCI_execute_command(command_table, prdt_entry_count, port);

	return;

}

/*
Function: 		AHCI_read
Description: 	Read 'sector_count' sectors starting from 'lba_high' << 0x20 + 'lba_low' disk connected on 'port'. 
				Doesn't allocate more than 0x1000 bytes for the command table.
				Cannot use all 65535 Physical Region Descriptor Table entries.
				Uses the ATA/ACS READ_DMA_EXT 48-bit addressing mode command.
Return:			NONE
*/
void AHCI_read(uint32_t port, void* buf, uint32_t lba_low, uint32_t lba_high, uint32_t sector_count) {

	AHCI_DEVICE_LIST* device = g_ahci_device_list;
	while (device != NULL) {
		if (device->port == port) {
			break;
		}
		device = device->next;
	}

	AHCI_COMMAND_TABLE* command_table = (AHCI_COMMAND_TABLE*)kalloc(1);
	memset((void*)command_table, 0x0, sizeof(AHCI_COMMAND_TABLE));

	SATA_REG_H2D_FIS* cfis = (SATA_REG_H2D_FIS*)(&command_table->CFIS);	
	
	cfis->FISType 		= FIS_TYPE_REG_H2D;
	cfis->Misc			= 0x80;
	cfis->Command 		= ATA_COMMAND_READ_DMA_EXT;

	cfis->LBA0			= (uint8_t)(lba_low >> 0x00);
	cfis->LBA1			= (uint8_t)(lba_low >> 0x08);
	cfis->LBA2			= (uint8_t)(lba_low >> 0x10);
	cfis->LBA3			= (uint8_t)(lba_low >> 0x18);
	cfis->LBA4			= (uint8_t)(lba_high >> 0x0);
	cfis->LBA5			= (uint8_t)(lba_high >> 0x8);

	cfis->SectorCount 		= (uint8_t)(sector_count >> 0x0);
	cfis->SectorCountExp	= (uint8_t)(sector_count >> 0x8);
	cfis->Device			= 0x40;

	AHCI_PRDT_ENTRY* prdt = (AHCI_PRDT_ENTRY*)(&command_table->PRDT);
	uint32_t prdt_entry_count = 0x0;
	
	uint8_t* buf_u8 = (uint8_t*)buf;
	uint8_t* buf_end_u8 = (uint8_t*)(buf + sector_count * device->lba_size);
	uint32_t write_length = 0x0;

	while (buf_u8 < buf_end_u8) {
		if ((buf_end_u8 - buf_u8) > 0x2000) {
			write_length = 0x2000;
		} 
		else {
			write_length = buf_end_u8 - buf_u8;
		}

		memset((void*)prdt, 0x0, sizeof(AHCI_PRDT_ENTRY));

		prdt->DBA	= (uint32_t)buf_u8;
		prdt->DBC 	= write_length - 1;
		prdt->DBC	|= (1 << 0x1F);

		prdt_entry_count++;
		prdt++;
		buf_u8 += write_length;

	}

	AHCI_execute_command(command_table, prdt_entry_count, port);

	return;

}

/*
Function: 		interrupt_function_ahci
Description: 	Interrupt handler for HBA generated interrupts. 
Return:			NONE
*/
void interrupt_function_ahci() {

	*(g_VGABuffer + VGA_WIDTH * 2 + 0x4F) += 1;

	for (uint8_t i = 0; i < 32; ++i) {
		if ((1 << i) & g_ahci_ghc->IS) {

			AHCI_PORT_SPECIFIC_REGISTERS* port_spec_regs = (void*)g_ahci_ghc + 0x100 + i*0x80;			
			port_spec_regs->PxIS = ~0x0;

		}
	}

	iowriteb(0x20, 0x20);
	iowriteb(0xA0, 0x20);

	return;

}

