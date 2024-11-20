#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

size_t strlen(const char* s) {

	size_t len = 0x0;

	while (*s != 0x00) {

		len++;
		s++;

	}

	return len;

}

uint8_t strncmp(const char* str1, const char* str2, size_t num) {

	for (size_t i = 0; i < num; ++i) {

		if (*str1 != *str2) {
			return 0x1;
		}

		str1++;
		str2++;

	}
	
	return 0x0;

}