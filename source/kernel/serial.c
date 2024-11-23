#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "io.h"
#include "memory.h"
#include "print.h"
#include "serial.h"
#include "string.h"

#define PORT_COM1 0x3F8

uint8_t transmit_status();

void serial_init() {

	disable_interrupts();

	iowriteb(PORT_COM1 + 1, 0x00);
	iowriteb(PORT_COM1 + 3, 0x80);
	iowriteb(PORT_COM1 + 0, 0x03);
	iowriteb(PORT_COM1 + 1, 0x00);
	iowriteb(PORT_COM1 + 3, 0x03);
	iowriteb(PORT_COM1 + 2, 0xC7);
	iowriteb(PORT_COM1 + 4, 0x0B);
	iowriteb(PORT_COM1 + 4, 0x1E);
	iowriteb(PORT_COM1 + 0, 0xAE);
	uint8_t comtest = ioreadb(PORT_COM1 + 0);
	if (comtest != 0xAE) {
		print_string("Serial COM1 failed loopback test.");
		print_newline();
		return;
	}

	iowriteb(PORT_COM1 + 4, 0x0F);

	enable_interrupts();

	return;

}

void serial_write_string(const char* str) {

	size_t string_len = strlen(str);
	for (size_t i = 0; i < string_len; ++i) {
		serial_write_char(str[i]); 
	}
	
	return;

}

void serial_write_dword(uint32_t n32) {

	serial_write_word((uint16_t)(n32 >> 0x10));
	serial_write_word((uint16_t)n32);

	return;

}

void serial_write_word(uint16_t n16) {

	serial_write_byte((uint8_t)(n16 >> 0x8));
	serial_write_byte((uint8_t)n16);

	return;

}

void serial_write_byte(uint8_t n8) {

	uint8_t lowBits = n8 & 0x0F;
	uint8_t highBits = (n8 & 0xF0) >> 4;

	if (lowBits < 0xA) lowBits += 0x30; else lowBits += 0x37;
	if (highBits < 0xA) highBits += 0x30; else highBits += 0x37;

	serial_write_char(highBits);
	serial_write_char(lowBits);

	return;

}

void serial_write_newline() {

	serial_write_char(0xA);

	return;

}

void serial_write_char(uint8_t c) {

	while (transmit_status() == 0);
	iowriteb(PORT_COM1, c);

	return;

}

uint8_t transmit_status() {

	return ioreadb(PORT_COM1 + 5) & 0x20;

}