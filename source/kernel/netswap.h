#ifndef NETSWAP_H
#define NETSWAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static inline uint16_t netswap16(uint16_t value) {

	uint8_t low = value;
	uint8_t high = value >> 0x8;

	return (low << 0x8 | high);

}

static inline uint32_t netswap32(uint32_t value) {

	uint8_t low = value;
	uint8_t mid_low = value >> 0x8;
	uint8_t mid_high = value >> 0x10;
	uint8_t high = value >> 0x18;

	return (low << 0x18 | mid_low << 0x10 | mid_high << 0x08 | high);
}

#endif /* NETSWAP_H */