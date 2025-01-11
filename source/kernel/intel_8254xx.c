#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "intel_8254xx.h"
#include "pci.h"
#include "time.h"
#include "mmio.h"
#include "networking.h"
#include "interrupt.h"
#include "memory.h"
#include "scheduler.h"
#include "serial.h"
#include "packet.h"
#include "list.h"
#include "ethernet.h"

#define REG_CTRL 0x00
#define REG_STATUS 0x08
#define REG_EECD 0x10
#define REG_EERD 0x14
#define REG_CTRL_EXT 0x18
#define REG_RCTL 0x100
#define REG_TCTL 0x400
#define REG_RDBAL 0x2800
#define REG_RDBAH 0x2804
#define REG_RDLEN 0x2808
#define REG_RDH 0x2810
#define REG_RDT 0x2818
#define REG_TDBAL 0x3800
#define REG_TDBAH 0x3804
#define REG_TDLEN 0x3808
#define REG_TDH 0x3810
#define REG_TDT 0x3818
#define REG_MTA0 0x5200
#define REG_RAL0 0x5400
#define REG_RAH0 0x5404

#define CTRL_SLU_SHIFT 6
#define CTRL_RST_SHIFT 26

#define EERD_START_SHIFT 0
#define EERD_DONE_SHIFT 4

#define RCTL_EN_SHIFT 1
#define RCTL_SBP_SHIFT 2
#define RCTL_UPE_SHIFT 3
#define RCTL_MPE_SHIFT 4
#define RCTL_LPE_SHIFT 5
#define RCTL_LBM_SHIFT 6
#define RCTL_RDMTS_SHIFT 8
#define RCTL_MO_SHIFT 12
#define RCTL_BAM_SHIFT 15
#define RCTL_BSIZE_SHIFT 16
#define RCTL_VFE_SHIFT 18
#define RCTL_CFIEN_SHIFT 19
#define RCTL_CFI_SHIFT 20
#define RCTL_DPF_SHIFT 22
#define RCTL_PMCF_SHIFT 23
#define RCTL_BSEX_SHIFT 25
#define RCTL_SECRC_SHIFT 26 

#define TCTL_EN_SHIFT 1
#define TCTL_PSP_SHIFT 3
#define TCTL_CT_SHIFT 4
#define TCTL_COLD_SHIFT 12
#define TCTL_RTLC_SHIFT 24

#define INTEL_8254XX_RX_DESC_COUNT 0x100
#define INTEL_8254XX_TX_DESC_COUNT 0x100

typedef struct __attribute__((__packed__)) INTEL_8254XX_RX_DESC {

	volatile uint32_t addr_low;
	volatile uint32_t addr_high;
	volatile uint16_t length;
	volatile uint16_t checksum;
	volatile uint8_t status;
	volatile uint8_t errors; 
	volatile uint16_t special;

} INTEL_8254XX_RX_DESC;

typedef struct __attribute__((__packed__)) INTEL_8254XX_TX_DESC {

	volatile uint32_t addr_low;
	volatile uint32_t addr_high;
	volatile uint16_t length;
	volatile uint8_t cso;
	volatile uint8_t command;
	volatile uint8_t status; 
	volatile uint8_t css;
	volatile uint16_t special;

} INTEL_8254XX_TX_DESC;

typedef struct __attribute__((__packed__)) INTEL_8254XX_DEVICE {

	INTEL_8254XX_RX_DESC* rxdl;
	INTEL_8254XX_TX_DESC* txdl;
	uint32_t tx_tail;
	uint32_t reg_base_addr;
	uint32_t rar_count;
	void(*rar_add)(void* device, void* eth_addr);

} INTEL_8254XX_DEVICE;

uint16_t intel_8254xx_eeprom_read(INTEL_8254XX_DEVICE* device, uint8_t addr);
void intel_8254xx_rx_init(INTEL_8254XX_DEVICE* device);
void intel_8254xx_tx_init(INTEL_8254XX_DEVICE* device);
void intel_8254xx_rx_poll(PPARAM device);
void intel_8254xx_tx_send(void* device, NETWORK_PACKET* pkt);
void intel_8254xx_rar_add(void* device, void* eth_addr);

ERR_CODE intel_8254xx_init(PCI_ENUM_TOKEN* pci_token) {

	// -- Allocate memory for network interface
	
	disable_interrupts();
	NETWORK_INTERFACE* intf = (NETWORK_INTERFACE*)kalloc(1);
	INTEL_8254XX_DEVICE* device = (INTEL_8254XX_DEVICE*)kalloc(1);
	enable_interrupts();

	intf->device = (void*)device;

	// -- Enable bus mastering; memory and IO access

	uint32_t pci_status_command = get_PCI_offset_token(pci_token, 0x4);
	pci_status_command |= 0x7;
	set_PCI_offset_token(pci_token, 0x4, pci_status_command);

	// -- Get memory mapped register base address

	uint32_t pci_bar0 = get_PCI_offset_token(pci_token, 0x10);	
	uint32_t mm_register_base_address = pci_bar0 & ~0x7;
	device->reg_base_addr = mm_register_base_address;

	// -- Reset the ethernet controller

	uint32_t ctrl = mmio_read(mm_register_base_address, REG_CTRL);
	ctrl |= 1 << CTRL_RST_SHIFT;
	mmio_write(mm_register_base_address, REG_CTRL, ctrl);
	sleep(1);
	while (mmio_read(mm_register_base_address, REG_CTRL) & (1 << CTRL_RST_SHIFT)) sleep(1);

	// -- Read EEPROM; MAC address

	uint16_t mac0 = intel_8254xx_eeprom_read(device, 0x0);
	uint16_t mac1 = intel_8254xx_eeprom_read(device, 0x1);
	uint16_t mac2 = intel_8254xx_eeprom_read(device, 0x2);

	intf->ether_addr[0] = (uint8_t)mac0;
	intf->ether_addr[1] = (uint8_t)(mac0 >> 0x8);
	intf->ether_addr[2] = (uint8_t)mac1;
	intf->ether_addr[3] = (uint8_t)(mac1 >> 0x8);
	intf->ether_addr[4] = (uint8_t)mac2;
	intf->ether_addr[5] = (uint8_t)(mac2 >> 0x8);

	// -- Set LINK UP

	ctrl = mmio_read(mm_register_base_address, REG_CTRL);
	ctrl |= CTRL_SLU_SHIFT;
	mmio_write(mm_register_base_address, REG_CTRL, ctrl);

	// -- Iniailize receive and transmit registers

	intel_8254xx_rx_init(device);
	intel_8254xx_tx_init(device);

	// -- Set rar_add function to device

	device->rar_count = 1;
	device->rar_add = intel_8254xx_rar_add;

	// -- Start polling process

	task_create_param(intel_8254xx_rx_poll, "Intel 8254xx RX Poll", (uint32_t)device);

	// -- Set tx_send for the network interface

	intf->tx_send = intel_8254xx_tx_send;

	// -- Set ip address, dhcp address, dns address, subnet mask to 0

	intf->ip_addr = 0;
	intf->ip_addr_dhcp = 0;
	intf->ip_addr_dns = 0;
	intf->subnet_mask = 0;

	// -- Add interface to list of network interfaces

	disable_interrupts();
	list_add(g_network_interfaces, (void*)intf);
	enable_interrupts();

	return E_OK;

}

uint16_t intel_8254xx_eeprom_read(INTEL_8254XX_DEVICE* device, uint8_t addr) {

	volatile uint32_t eerd = 0x0;
	eerd |= 1 << EERD_START_SHIFT;
	eerd |= addr << 8;
	mmio_write(device->reg_base_addr, REG_EERD, eerd);
	while (!(mmio_read(device->reg_base_addr, REG_EERD) & (1 << EERD_DONE_SHIFT))) sleep(1);

	eerd = mmio_read(device->reg_base_addr, REG_EERD);
	return eerd >> 0x10;

}

void intel_8254xx_rx_init(INTEL_8254XX_DEVICE* device) {

	// -- Clear the multicast table array

	for (size_t i = 0; i < 128; ++i) mmio_write(device->reg_base_addr, REG_MTA0 + i * 4, 0);

	// -- Allocate memory and initialize receive descriptors and receive buffers

	disable_interrupts();
	INTEL_8254XX_RX_DESC* rxdl = (INTEL_8254XX_RX_DESC*)kalloc(0x2);
	enable_interrupts();

	for (size_t i = 0; i < INTEL_8254XX_RX_DESC_COUNT; ++i) {

		memset(&rxdl[i], 0, sizeof(INTEL_8254XX_RX_DESC));
		disable_interrupts();
		rxdl[i].addr_low = (uint32_t)kalloc(0x1);
		enable_interrupts();

	}

	// -- Set registers

	mmio_write(device->reg_base_addr, REG_RDBAL, (uint32_t)rxdl);
	mmio_write(device->reg_base_addr, REG_RDBAH, 0x0);
	mmio_write(device->reg_base_addr, REG_RDLEN, INTEL_8254XX_RX_DESC_COUNT * sizeof(INTEL_8254XX_RX_DESC));
	mmio_write(device->reg_base_addr, REG_RDH, 0x0);
	mmio_write(device->reg_base_addr, REG_RDT, INTEL_8254XX_RX_DESC_COUNT);

	uint32_t rctl = 0x0;
	rctl |= 1 << RCTL_EN_SHIFT;
	rctl |= 1 << RCTL_SBP_SHIFT;
	rctl |= 1 << RCTL_UPE_SHIFT;
	rctl |= 1 << RCTL_MPE_SHIFT;
	rctl |= 1 << RCTL_BAM_SHIFT;
	rctl |= 0b11 << RCTL_BSIZE_SHIFT;
	rctl |= 1 << RCTL_BSEX_SHIFT;
	rctl |= 1 << RCTL_SECRC_SHIFT;

	mmio_write(device->reg_base_addr, REG_RCTL, rctl);

	device->rxdl = rxdl;

	return;

}

void intel_8254xx_tx_init(INTEL_8254XX_DEVICE* device) {

	// -- Allocate memory and intialize transmit descriptors

	disable_interrupts();
	INTEL_8254XX_TX_DESC* txdl = (INTEL_8254XX_TX_DESC*)kalloc(0x2);
	enable_interrupts();

	for (size_t i = 0; i < INTEL_8254XX_TX_DESC_COUNT; ++i) {
	
		memset(&txdl[i], 0, sizeof(INTEL_8254XX_TX_DESC));
		txdl[i].status = 1;

	}

	// -- Set registers

	mmio_write(device->reg_base_addr, REG_TDBAL, (uint32_t)txdl);
	mmio_write(device->reg_base_addr, REG_TDBAH, 0x0);
	mmio_write(device->reg_base_addr, REG_TDLEN, INTEL_8254XX_TX_DESC_COUNT * sizeof(INTEL_8254XX_TX_DESC));
	mmio_write(device->reg_base_addr, REG_TDH, 0x0);
	mmio_write(device->reg_base_addr, REG_TDT, 0x0);

	uint32_t tctl = 0x0;
	tctl |= 1 << TCTL_EN_SHIFT;
	tctl |= 1 << TCTL_PSP_SHIFT;
	tctl |= 15 << TCTL_CT_SHIFT;
	tctl |= 64 << TCTL_COLD_SHIFT;
	tctl |= 1 << TCTL_RTLC_SHIFT;

	mmio_write(device->reg_base_addr, REG_TCTL, tctl);

	device->tx_tail = 0;
	device->txdl = txdl;

	return;

}

void intel_8254xx_rx_poll(PPARAM device) {

	INTEL_8254XX_DEVICE* device_ptr = (INTEL_8254XX_DEVICE*)device;

	while (true) {

		for (size_t i = 0; i < INTEL_8254XX_RX_DESC_COUNT; ++i) {
			if (device_ptr->rxdl[i].status & 1) {
		
				disable_interrupts();
				serial_write_string("[INTEL_8254XX_RX_POLL] Packet received in descriptor 0x");
				serial_write_dword(i);
				serial_write_newline();
				enable_interrupts();

				disable_interrupts();
				NETWORK_MESSAGE* msg = (NETWORK_MESSAGE*)kalloc(1);
				void* msg_buf = kalloc(1);
				enable_interrupts();
				msg->buf = msg_buf;
				NETWORK_MESSAGE_DESC msg_desc = {};

				ERR_CODE error_code = ether_decode(msg, &msg_desc, (void*)device_ptr->rxdl[i].addr_low);
				if (error_code == E_OK) networking_rx_sort(msg, &msg_desc, (void*)device, device_ptr->rar_add);
				
				device_ptr->rxdl[i].status = 0;

			}
		}

		sleep(10);

	}

	return;

}

void intel_8254xx_tx_send(void* device, NETWORK_PACKET* pkt) {

	INTEL_8254XX_DEVICE* device_ptr = (INTEL_8254XX_DEVICE*)device;

	uint32_t tx_tail = device_ptr->tx_tail;
	INTEL_8254XX_TX_DESC* tx_desc = &device_ptr->txdl[tx_tail];

	tx_desc->addr_low = (uint32_t)pkt->start;
	tx_desc->length = pkt->end - pkt->start;
	tx_desc->command = 0xB;
	tx_desc->status = 0x0;

	device_ptr->tx_tail = (tx_tail + 1) % INTEL_8254XX_TX_DESC_COUNT;
	mmio_write(device_ptr->reg_base_addr, REG_TDT, device_ptr->tx_tail);

	return;

}

void intel_8254xx_rar_add(void* device, void* eth_addr) {

	INTEL_8254XX_DEVICE* device_ptr = (INTEL_8254XX_DEVICE*)device;

	uint8_t* eth_u8 = (uint8_t*)eth_addr;
	uint32_t ral = eth_u8[0] + (eth_u8[1] << 0x8) + (eth_u8[2] << 0x10) + (eth_u8[3] << 0x18); 
	uint32_t rah = eth_u8[4] + (eth_u8[5] << 0x8);
	rah |= 0x01 << 0x10;
	rah |= 1 << 0x1F;

	mmio_write(device_ptr->reg_base_addr, REG_RAL0 + 8 * device_ptr->rar_count, ral);
	mmio_write(device_ptr->reg_base_addr, REG_RAH0 + 8 * device_ptr->rar_count, rah);

	device_ptr->rar_count++;

	return;

}