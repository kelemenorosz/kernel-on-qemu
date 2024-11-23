#ifndef SERIAL_H
#define SERIAL_H

void serial_init();

void serial_write_char(uint8_t c);
void serial_write_string(const char* str);
void serial_write_word(uint16_t n16);
void serial_write_byte(uint8_t n8);
void serial_write_dword(uint32_t n32);
void serial_write_newline();

#endif /* SERIAL_H */