#ifndef SOCKET_H
#define SOCKET_H

#include "queue.h"
#include "networking.h"
#include "err.h"

#define SOCKET_PROTOCOL_TCP 0
#define SOCKET_PROTOCOL_UDP 1

typedef enum {

	S_SYN = 0,
	S_ACK = 1,
	S_FIN = 2,
	S_OPEN = 3,
	S_WRITE = 4,
	S_READ = 5,
	S_NULL = 6

} SOCKET_STATUS;

typedef struct __attribute__((__packed__)) SOCKET {

	uint32_t port_in;
	uint32_t port_out;
	uint32_t ip_out;
	uint32_t protocol;
	uint32_t seq;
	uint32_t ack;
	SOCKET_STATUS status;
	SOCKET_STATUS connect_status;
	NETWORK_INTERFACE* intf;
	QUEUE* message_queue_in;
	QUEUE* message_queue_out;

} SOCKET;

SOCKET* ksocket(NETWORK_INTERFACE* intf, uint32_t port_in);
ERR_CODE kconnect(SOCKET* sck, uint32_t ip_out, uint32_t port_out, uint32_t protocol);
void kclose(SOCKET* sck);
ERR_CODE kwrite(SOCKET* sck, void* buf, size_t len);
uint32_t kread(SOCKET* sck, void* buf);

#endif /* SOCKET_H */