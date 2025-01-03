#ifndef ETHERNET_H
#define ETHERNET_H

#include "packet.h"
#include "networking.h"
#include "err.h"

#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_ARP 0x0806

void ether_req(NETWORK_PACKET* pkt, NETWORK_INTERFACE* intf, uint16_t ether_type, uint32_t ip);
ERR_CODE ether_decode(NETWORK_MESSAGE* msg, NETWORK_MESSAGE_DESC* msg_desc, void* buf);

#endif /* ETHERNET_H */
























// #ifndef ETHERNET_H
// #define ETHERNET_H

// #include "pci.h"
// #include "packet.h"

// #define ETHERNET_TRANSPORT_PROTOCOL_UDP 0
// #define ETHERNET_TRANSPORT_PROTOCOL_TCP 1

// typedef struct __attribute__((__packed__)) RX_DESCRIPTOR {

// 	volatile uint32_t addr_low;
// 	volatile uint32_t addr_high;
// 	volatile uint16_t length;
// 	volatile uint16_t checksum;
// 	volatile uint8_t status;
// 	volatile uint8_t errors; 
// 	volatile uint16_t special;

// } RX_DESCRIPTOR;

// typedef struct __attribute__((__packed__)) TX_DESCRIPTOR {

// 	volatile uint32_t addr_low;
// 	volatile uint32_t addr_high;
// 	volatile uint16_t length;
// 	volatile uint8_t cso;
// 	volatile uint8_t command;
// 	volatile uint8_t status; 
// 	volatile uint8_t css;
// 	volatile uint16_t special;

// } TX_DESCRIPTOR;

// typedef struct __attribute__((__packed__)) ETHERNET_DEVICE {

// 	RX_DESCRIPTOR* rxdl;
// 	TX_DESCRIPTOR* txdl;
// 	uint32_t tx_tail;
// 	uint8_t mac_addr[6];

// } ETHERNET_DEVICE;

// // typedef struct __attribute__((__packed__)) SOCKET {

// // 	uint32_t ip;
// // 	uint32_t port;
// // 	uint32_t protocol;
// // 	uint32_t tcp_state;
// // 	uint32_t seq_number;
// // 	uint32_t ack_number;
// // 	uint32_t recv_size;
// // 	void* recv_buffer;
// // 	struct SOCKET* sck_next;

// // } SOCKET;

// // #define TCP_STATE_NONE 0

// typedef struct __attribute__((__packed__)) SOCKET_LIST {

// 	// SOCKET* sck_head;

// } SOCKET_LIST;

// #define ETHER_TYPE_IPV4 0x0800
// #define ETHER_TYPE_ARP 0x0806

// // void ethernet_init(PCI_ENUM_TOKEN* token);
// // SOCKET* ksocket(uint32_t ip, uint32_t port, uint32_t protocol); 
// // void ksend(SOCKET* sck, void* buf, uint32_t buf_len, uint32_t send_port, uint32_t ip);
// // void* kreceive(SOCKET* sck);
// // uint8_t kconnect(SOCKET* sck, uint32_t ip);
// // void kwrite(SOCKET* sck, void* buf, uint32_t buf_len, uint32_t ip);
// // uint32_t kread(SOCKET* sck, void* buf);
// // void kclose(SOCKET* sck);

// // void ether_req(NET_PACKET* pkt, uint16_t ether_type, uint32_t ip);
// // uint32_t ether_decode(void* buf);
// // void tx_send(void* buf, uint16_t len);

// void rar_add(void* eth);
// void rar_read(size_t index);

// #endif /* ETHERNET_H */