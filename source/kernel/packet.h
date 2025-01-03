#ifndef PACKET_H
#define PACKET_H

typedef struct __attribute__((__packed__)) NETWORK_PACKET {

	void* start;
	void* end;

} NETWORK_PACKET;

#endif /* PACKET_H */