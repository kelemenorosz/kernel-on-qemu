#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t netchecksum_acc(uint8_t* data, uint8_t* end, uint32_t sum);
uint16_t netchecksum_final(uint32_t sum);

uint16_t netchecksum(uint8_t* data, uint8_t* end) {

	uint32_t sum = netchecksum_acc(data, end, 0);
	return netchecksum_final(sum);

}

uint32_t netchecksum_acc(uint8_t* data, uint8_t* end, uint32_t sum) {

	uint32_t len = end - data;
	uint16_t* p = (uint16_t*)data;

	while (len > 1) {
		sum += *p++;
		len -= 2;
	}

	if (len) {
		sum += *(uint8_t*)p;
	}

	return sum;

}

uint16_t netchecksum_final(uint32_t sum) {

	sum = (sum & 0xFFFF) + (sum >> 16);
	sum += (sum >> 16);

	uint16_t temp = ~sum;
	return ((temp & 0x00FF) << 8) | ((temp & 0xFF00) >> 8);

}