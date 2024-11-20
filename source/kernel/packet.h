#ifndef PACKET_H
#define PACKET_H

typedef struct __attribute__((__packed__)) NET_PACKET {

	void* start;
	void* end;

} NET_PACKET;

#endif /* PACKET_H */