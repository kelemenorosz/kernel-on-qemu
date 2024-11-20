#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"
#include "string.h"

extern size_t g_consoleIndex;
extern size_t g_consoleRow;
extern uint16_t* const g_VGABuffer;
extern const size_t VGA_WIDTH;
extern const size_t VGA_HEIGHT;

void print_byte(const uint8_t to_print) {

	uint8_t lowBits = to_print & 0x0F;
	uint8_t highBits = (to_print & 0xF0) >> 4;

	if (lowBits < 0xA) lowBits += 0x30; else lowBits += 0x37;
	if (highBits < 0xA) highBits += 0x30; else highBits += 0x37;

	g_VGABuffer[g_consoleIndex] = 0x0F00 + highBits;
	g_VGABuffer[g_consoleIndex + 1] = 0x0F00 + lowBits;

	g_consoleIndex += 0x2;

	return;

}

void print_word(const uint16_t to_print) {

	print_byte((uint8_t)(to_print >> 0x8));
	print_byte((uint8_t)to_print);

	return;

}

void print_dword(const uint32_t to_print) {

	print_word((uint16_t)(to_print >> 0x10));
	print_word((uint16_t)to_print);

	return;

}

void print_space(const uint32_t space_len) {

	for (size_t i = 0; i < space_len; ++i) {
		g_VGABuffer[g_consoleIndex] = 0x0F20;
		g_consoleIndex += 1;
	}

	return;

}

void print_string(const char* const string) {

	size_t string_len = strlen(string);
	for (size_t i = 0; i < string_len; ++i) {
		g_VGABuffer[g_consoleIndex] = 0x0F00 | string[i]; 
		g_consoleIndex += 1;
	}

	return;

}

void print_newline() {

	g_consoleRow++;
	g_consoleIndex = g_consoleRow * VGA_WIDTH; 

	return;

}

void cls() {

	for (size_t i = 0; i < VGA_HEIGHT; ++i) {
		for (size_t j = 0; j < VGA_WIDTH; ++j) {
			const size_t index = i * VGA_WIDTH + j;
			g_VGABuffer[index] = 0x0F20;
		}
	}

	return;

}