#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ethernet.h"
#include "io.h"
#include "pci.h"
#include "print.h"
#include "time.h"
#include "memory.h"
#include "interrupt.h"
#include "dhcp.h"
#include "packet.h"
#include "netswap.h"
#include "serial.h"
#include "udp.h"
#include "packet.h"
#include "ipv4.h"

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

#define EECD_REQ_SHIFT 6
#define EECD_GNT_SHIFT 7

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

#define RX_DESCRIPTOR_COUNT 0x100
#define TX_DESCRIPTOR_COUNT 0x100

uint32_t g_reg_base_addr = 0x0;

uint32_t mmio_read(uint32_t addr, uint32_t offset);
void mmio_write(uint32_t addr, uint32_t offset, uint32_t value);

void lock_eeprom();
void unclock_eeprom();

uint16_t eeprom_read_eerd(uint8_t addr);

void rx_init();
void tx_init();

void tx_send(void* buf, uint16_t len);

extern void interrupt_wrapper_ethernet();
void interrupt_function_ethernet();

void print_TX_DESCRIPTOR(TX_DESCRIPTOR* tx_desc);
void print_RX_DESCRIPTOR(RX_DESCRIPTOR* rx_desc);
void print_tx_regs();
void print_rx_regs();

void rx_poll();

extern uint16_t* g_VGABuffer;
extern size_t VGA_WIDTH;

typedef struct __attribute__((__packed__)) ETHER_HEADER {

	uint8_t dest_addr[6];
	uint8_t src_addr[6];
	uint16_t ether_type;

} ETHER_HEADER;

SOCKET_LIST* g_sck_list = NULL;

ETHERNET_DEVICE g_ethernet_device = {};

void ethernet_init(PCI_ENUM_TOKEN* token) {

	// -- Enable bus mastering; memory and IO access

	uint32_t pci_status_command = get_PCI_offset_token(token, 0x4);
	pci_status_command |= 0x7;
	set_PCI_offset_token(token, 0x4, pci_status_command);

	// -- Get memory mapped register base address

	uint32_t pci_bar0 = get_PCI_offset_token(token, 0x10);	
	uint32_t mm_register_base_address = pci_bar0 & ~0x7;
	g_reg_base_addr = mm_register_base_address;

	disable_interrupts();
	print_string("PCI BAR0: ");
	print_dword(pci_bar0);
	print_newline();
	enable_interrupts();

	// -- Reset the ethernet controller

	uint32_t ctrl = mmio_read(mm_register_base_address, REG_CTRL);
	ctrl |= 1 << CTRL_RST_SHIFT;
	mmio_write(mm_register_base_address, REG_CTRL, ctrl);
	sleep(1);
	while (mmio_read(mm_register_base_address, REG_CTRL) & (1 << CTRL_RST_SHIFT)) sleep(1);

	// -- Read EEPROM; MAC address

	uint16_t mac0 = eeprom_read_eerd(0x0);
	uint16_t mac1 = eeprom_read_eerd(0x1);
	uint16_t mac2 = eeprom_read_eerd(0x2);

	g_ethernet_device.mac_addr[0] = (uint8_t)mac0;
	g_ethernet_device.mac_addr[1] = (uint8_t)(mac0 >> 0x8);
	g_ethernet_device.mac_addr[2] = (uint8_t)mac1;
	g_ethernet_device.mac_addr[3] = (uint8_t)(mac1 >> 0x8);
	g_ethernet_device.mac_addr[4] = (uint8_t)mac2;
	g_ethernet_device.mac_addr[5] = (uint8_t)(mac2 >> 0x8);

	// -- Set LINK UP

	ctrl = mmio_read(mm_register_base_address, REG_CTRL);
	ctrl |= CTRL_SLU_SHIFT;
	mmio_write(mm_register_base_address, REG_CTRL, ctrl);

	// -- Enable interrupts

	uint8_t interrupt_line = (uint8_t)get_PCI_offset(token->bus, token->device, token->function, 0x3C); 
	register_interrupt(interrupt_wrapper_ethernet, interrupt_line);
	PIC_line_enable(interrupt_line);

	mmio_read(mm_register_base_address, 0xC0);

	// -- Receive and transmit initialization

	rx_init();
	tx_init();

	// -- Setup socket list
	disable_interrupts();
	g_sck_list = (SOCKET_LIST*)kalloc(1);
	enable_interrupts();
	g_sck_list->sck_head = NULL;

	// -- Setup polling process

	task_create(rx_poll);

	// -- Transmit packets

	// disable_interrupts();
	// serial_write_string("[ETHERNET] Transmitting packets.");
	// serial_write_newline();
	// enable_interrupts();

	// disable_interrupts();
	// void* transmit_buffer = kalloc(4);
	// enable_interrupts();

	// NET_PACKET packet = {};
	// packet.start = transmit_buffer + 0xFF;
	// packet.end = transmit_buffer + 0xFF;

	// req_dhcp(&g_ethernet_device, &packet);

	// packet.start -= sizeof(ETHERNET_HEADER);
	// ETHERNET_HEADER* ethernet_header = (ETHERNET_HEADER*)packet.start;

	// memset((void*)&ethernet_header->dest_addr[0], 0xFF, 6);
	// memcpy((void*)&ethernet_header->src_addr[0], (void*)&g_ethernet_device.mac_addr[0], 6);
	// ethernet_header->ether_type = netswap16(ETHER_TYPE_IPV4);

	// disable_interrupts();
	// serial_write_string("[ETHERNET] Packet AT 0x");
	// serial_write_dword((uint32_t)transmit_buffer);
	// serial_write_string(" assembled.");
	// serial_write_newline();
	// enable_interrupts();

	// tx_send(packet.start, packet.end - packet.start);

	// print_RX_DESCRIPTOR(g_ethernet_device.rxdl + 2);

	// while (1) {

	// 	sleep(100);
	// 	rx_poll();
	// 	tx_send(packet.start, packet.end - packet.start);

	// }

	return;

}

void interrupt_function_ethernet() {

	*(g_VGABuffer + VGA_WIDTH * 3 + 0x4F) += 1;

	iowriteb(0x20, 0x20);
	iowriteb(0xA0, 0x20);

	return;

}

void ether_req(NET_PACKET* pkt, uint16_t ether_type) {

	pkt->start -= sizeof(ETHER_HEADER);
	ETHER_HEADER* ether_header = (ETHER_HEADER*)pkt->start;

	memset((void*)&ether_header->dest_addr[0], 0xFF, 6);
	memcpy((void*)&ether_header->src_addr[0], (void*)&g_ethernet_device.mac_addr[0], 6);
	ether_header->ether_type = netswap16(ether_type);

	return;

}

SOCKET* ksocket(uint32_t ip, uint32_t port, uint32_t protocol) {

	disable_interrupts();
	SOCKET* sck = (SOCKET*)kalloc(1);
	enable_interrupts();

	sck->ip = ip;
	sck->port = port;
	sck->protocol = protocol;
	sck->recv_buffer = NULL;

	sck->sck_next = g_sck_list->sck_head;
	g_sck_list->sck_head = sck;

	return sck;

}

void* kreceive(SOCKET* sck) {

	while (sck->recv_buffer == NULL) sleep(1);
	return sck->recv_buffer;

}

/* 
Function: 		ksend
Description: 	Prepares packet to be sent.
				Receives buffer with data, wraps it in TCP/UDP, IP and ETH layers.
Return:			NONE
*/
void ksend(SOCKET* sck, void* buf, uint32_t buf_len, uint32_t send_port, uint32_t ip) {

	// -- Allocate buffer
	
	disable_interrupts();
	void* wrap_buf = kalloc(1);
	enable_interrupts();

	// -- Copy to wrap buffer

	memcpy(wrap_buf + 0x100, buf, buf_len);
	NET_PACKET pkt = {wrap_buf + 0x100, wrap_buf + 0x100 + buf_len};

	// -- Package

	if (sck->protocol == ETHERNET_TRANSPORT_PROTOCOL_UDP) {

		udp_req(&pkt, send_port, sck->port, ip);

	}

	tx_send(pkt.start, pkt.end - pkt.start);

	return;

}

uint32_t ether_decode(void* buf) {

	ETHER_HEADER* ether_header = (ETHER_HEADER*)buf;

	disable_interrupts();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with destination mac address ");
	for (uint8_t i = 0; i < 6; ++i) {
		serial_write_byte(ether_header->dest_addr[i]);
		if (i != 5) serial_write_string(":");
	}
	serial_write_newline();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with source mac address ");
	for (uint8_t i = 0; i < 6; ++i) {
		serial_write_byte(ether_header->src_addr[i]);
		if (i != 5) serial_write_string(":");
	}
	serial_write_newline();
	serial_write_string("[RX_ETH_HEADER] Ethernet header with ether type 0x");
	serial_write_word(netswap16(ether_header->ether_type));
	serial_write_newline();
	enable_interrupts();

	if (netswap16(ether_header->ether_type) == ETHER_TYPE_IPV4) {

		return ipv4_decode(buf + sizeof(ETHER_HEADER));

	}

	return 0;

}

/* 
Function: 		rx_poll
Description: 	Ethernet RX polling process.
Return:			NONE
*/
void rx_poll() {

	while (true) {

		for (size_t i = 0; i < RX_DESCRIPTOR_COUNT; ++i) {
			if (g_ethernet_device.rxdl[i].status & 1) {
		
				disable_interrupts();
				serial_write_string("[RX_POLL] Packet received in descriptor 0x");
				serial_write_dword(i);
				serial_write_newline();
				enable_interrupts();

				if (!ether_decode((void*)g_ethernet_device.rxdl[i].addr_low)) g_ethernet_device.rxdl[i].status = 0;

			}
		}

		// print_rx_regs();

		sleep(10);

	}

	return;

}

void tx_send(void* buf, uint16_t len) {

	// print_tx_regs();

	uint32_t tx_tail = g_ethernet_device.tx_tail;
	TX_DESCRIPTOR* tx_desc = &g_ethernet_device.txdl[tx_tail];

	tx_desc->addr_low = (uint32_t)buf;
	tx_desc->length = len;
	tx_desc->command = 0xB;
	tx_desc->status = 0x0;

	// print_string("TX DESC addr: ");
	// print_dword((uint32_t)tx_desc);
	// print_newline();

	// print_TX_DESCRIPTOR(tx_desc);

	g_ethernet_device.tx_tail = (tx_tail + 1) % TX_DESCRIPTOR_COUNT;
	mmio_write(g_reg_base_addr, REG_TDT, g_ethernet_device.tx_tail);

	sleep(5);

	return;

}

void tx_init() {

	// -- Set up the transmit descriptor list; descriptor count: TX_DESCRIPTOR_COUNT
	// 8K for the descriptor list

	// print_string("MMIO base address: ");
	// print_dword(g_reg_base_addr);
	// print_newline();

	// print_tx_regs();

	disable_interrupts();
	TX_DESCRIPTOR* txdl = (TX_DESCRIPTOR*)kalloc(0x2);
	enable_interrupts();

	for (size_t i = 0; i < TX_DESCRIPTOR_COUNT; ++i) {
	
		memset(&txdl[i], 0, sizeof(TX_DESCRIPTOR));
		txdl[i].status = 1;

		// print_string("TX DESC addr: ");
		// print_dword((uint32_t)&txdl[i]);
		// print_newline();

	}
		
	// print_TX_DESCRIPTOR(txdl);

	// -- Set registers
	mmio_write(g_reg_base_addr, REG_TDBAL, (uint32_t)txdl);
	mmio_write(g_reg_base_addr, REG_TDBAH, 0x0);
	mmio_write(g_reg_base_addr, REG_TDLEN, TX_DESCRIPTOR_COUNT * sizeof(TX_DESCRIPTOR));
	mmio_write(g_reg_base_addr, REG_TDH, 0x0);
	mmio_write(g_reg_base_addr, REG_TDT, 0x0);

	uint32_t tctl = 0x0;
	tctl |= 1 << TCTL_EN_SHIFT;
	tctl |= 1 << TCTL_PSP_SHIFT;
	tctl |= 15 << 4;
	tctl |= 64 << 12;
	tctl |= 1 << 24;

	mmio_write(g_reg_base_addr, REG_TCTL, tctl);

	g_ethernet_device.tx_tail = 0;
	g_ethernet_device.txdl = txdl;

	// print_tx_regs();

	return;

}

void rx_init() {

	// print_rx_regs();

	// -- Clear the multicast table array

	for (size_t i = 0; i < 128; ++i) mmio_write(g_reg_base_addr, REG_MTA0 + i * 4, 0);

	// -- Set up the receive descriptor list; descriptor count: RX_DESCRIPTOR_COUNT
	// 8K for the descriptor list
	// 4k for each descriptor buffer

	disable_interrupts();
	RX_DESCRIPTOR* rxdl = (RX_DESCRIPTOR*)kalloc(0x2);
	enable_interrupts();

	for (size_t i = 0; i < RX_DESCRIPTOR_COUNT; ++i) {

		memset(&rxdl[i], 0, sizeof(RX_DESCRIPTOR));
		disable_interrupts();
		rxdl[i].addr_low = (uint32_t)kalloc(0x1);
		enable_interrupts();

	}
	
	// -- Set registers
	mmio_write(g_reg_base_addr, REG_RDBAL, (uint32_t)rxdl);
	mmio_write(g_reg_base_addr, REG_RDBAH, 0x0);
	mmio_write(g_reg_base_addr, REG_RDLEN, RX_DESCRIPTOR_COUNT * sizeof(RX_DESCRIPTOR));
	mmio_write(g_reg_base_addr, REG_RDH, 0x0);
	mmio_write(g_reg_base_addr, REG_RDT, RX_DESCRIPTOR_COUNT);

	uint32_t rctl = 0x0;
	rctl |= 1 << RCTL_EN_SHIFT;
	rctl |= 1 << RCTL_SBP_SHIFT;
	rctl |= 1 << RCTL_UPE_SHIFT;
	rctl |= 1 << RCTL_MPE_SHIFT;
	// rctl |= 1 << RCTL_LPE_SHIFT;
	rctl |= 1 << RCTL_BAM_SHIFT;
	rctl |= 0b11 << RCTL_BSIZE_SHIFT;
	rctl |= 1 << RCTL_BSEX_SHIFT;
	rctl |= 1 << RCTL_SECRC_SHIFT;

	mmio_write(g_reg_base_addr, REG_RCTL, rctl);

	g_ethernet_device.rxdl = rxdl;

	// print_rx_regs();

	return;

}

uint16_t eeprom_read_eerd(uint8_t addr) {

	volatile uint32_t eerd = 0x0;
	eerd |= 1 << EERD_START_SHIFT;
	eerd |= addr << 8;
	mmio_write(g_reg_base_addr, REG_EERD, eerd);
	while (!(mmio_read(g_reg_base_addr, REG_EERD) & (1 << EERD_DONE_SHIFT))) sleep(1);

	eerd = mmio_read(g_reg_base_addr, REG_EERD);
	return eerd >> 0x10;

}

void lock_eeprom() {

	uint32_t eecd = mmio_read(g_reg_base_addr, REG_EECD);
	eecd |= 1 << EECD_REQ_SHIFT;
	mmio_write(g_reg_base_addr, REG_EECD, eecd);
	while (!(mmio_read(g_reg_base_addr, REG_EECD) & (1 << EECD_GNT_SHIFT)));

	return;

}

void unclock_eeprom() {

	uint32_t eecd = mmio_read(g_reg_base_addr, REG_EECD);
	eecd &= ~(1 << EECD_REQ_SHIFT);
	mmio_write(g_reg_base_addr, REG_EECD, eecd);

	return;

}

uint32_t mmio_read(uint32_t addr, uint32_t offset) {

	return *(volatile uint32_t*)(addr + offset);

}

void mmio_write(uint32_t addr, uint32_t offset, uint32_t value) {

	*(volatile uint32_t*)(addr + offset) = value;

	return;

}

void print_TX_DESCRIPTOR(TX_DESCRIPTOR* tx_desc) {

	disable_interrupts();
	print_string("TX DESC addr_low: ");
	print_dword(tx_desc->addr_low);
	print_newline();

	print_string("TX DESC addr_high: ");
	print_dword(tx_desc->addr_high);
	print_newline();
	
	print_string("TX DESC length: ");
	print_word(tx_desc->length);
	print_newline();
	
	print_string("TX DESC cso: ");
	print_byte(tx_desc->cso);
	print_newline();
	
	print_string("TX DESC command: ");
	print_byte(tx_desc->command);
	print_newline();

	print_string("TX DESC status: ");
	print_byte(tx_desc->status);
	print_newline();
	
	print_string("TX DESC css: ");
	print_byte(tx_desc->css);
	print_newline();

	print_string("TX DESC special: ");
	print_word(tx_desc->special);
	print_newline();
	enable_interrupts();

	return;

}

void print_RX_DESCRIPTOR(RX_DESCRIPTOR* rx_desc) {

	disable_interrupts();
	print_string("RX DESC addr_low: ");
	print_dword(rx_desc->addr_low);
	print_newline();

	print_string("RX DESC addr_high: ");
	print_dword(rx_desc->addr_high);
	print_newline();

	print_string("RX DESC length: ");
	print_word(rx_desc->length);
	print_newline();
	
	print_string("RX DESC checksum: ");
	print_word(rx_desc->checksum);
	print_newline();

	print_string("RX DESC status: ");
	print_byte(rx_desc->status);
	print_newline();
	
	print_string("RX DESC errors: ");
	print_byte(rx_desc->errors);
	print_newline();

	print_string("RX DESC special: ");
	print_word(rx_desc->special);
	print_newline();
	enable_interrupts();

	return;

}

void print_tx_regs() {

	uint32_t tdbal = mmio_read(g_reg_base_addr, REG_TDBAL);
	uint32_t tdbah = mmio_read(g_reg_base_addr, REG_TDBAH);
	uint32_t tdlen = mmio_read(g_reg_base_addr, REG_TDLEN);
	uint32_t tdh = mmio_read(g_reg_base_addr, REG_TDH);
	uint32_t tdt = mmio_read(g_reg_base_addr, REG_TDT);

	disable_interrupts();
	print_string("TDBAL: ");
	print_dword(tdbal);
	print_newline();
	
	print_string("TDBAH: ");
	print_dword(tdbah);
	print_newline();
	
	print_string("TDLEN: ");
	print_dword(tdlen);
	print_newline();
	
	print_string("TDH: ");
	print_dword(tdh);
	print_newline();

	print_string("TDT: ");
	print_dword(tdt);
	print_newline();
	enable_interrupts();

	return;

}

void print_rx_regs() {

	uint32_t rdbal = mmio_read(g_reg_base_addr, REG_RDBAL);
	uint32_t rdbah = mmio_read(g_reg_base_addr, REG_RDBAH);
	uint32_t rdlen = mmio_read(g_reg_base_addr, REG_RDLEN);
	uint32_t rdh = mmio_read(g_reg_base_addr, REG_RDH);
	uint32_t rdt = mmio_read(g_reg_base_addr, REG_RDT);

	disable_interrupts();
	print_string("RDBAL: ");
	print_dword(rdbal);
	print_newline();
	
	print_string("RDBAH: ");
	print_dword(rdbah);
	print_newline();
	
	print_string("RDLEN: ");
	print_dword(rdlen);
	print_newline();
	
	print_string("RDH: ");
	print_dword(rdh);
	print_newline();

	print_string("RDT: ");
	print_dword(rdt);
	print_newline();
	enable_interrupts();

	return;

}