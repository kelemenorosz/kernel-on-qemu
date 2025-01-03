#ifndef SOCKET_H
#define SOCKET_H

#include "queue.h"
#include "networking.h"
#include "err.h"

#define SOCKET_PROTOCOL_TCP 0
#define SOCKET_PROTOCOL_UDP 1

typedef struct __attribute__((__packed__)) SOCKET {

	uint32_t port_in;
	uint32_t port_out;
	uint32_t ip_out;
	uint32_t protocol;
	NETWORK_INTERFACE* intf;
	QUEUE* message_queue_in;
	QUEUE* message_queue_out;

} SOCKET;

SOCKET* ksocket(NETWORK_INTERFACE* intf, uint32_t port_in);
ERR_CODE kconnect(SOCKET* sck, uint32_t ip_out, uint32_t port_out, uint32_t protocol);
ERR_CODE kwrite(SOCKET* sck, void* buf, size_t len);
uint32_t kread(SOCKET* sck, void* buf);

#endif /* SOCKET_H */