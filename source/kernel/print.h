#ifndef PRINT_H
#define PRINT_H

void print_dword(const uint32_t to_print);
void print_word(const uint16_t to_print);
void print_byte(const uint8_t to_print);
void print_space(const uint32_t space_len);
void print_string(const char* const string);
void print_newline();

void cls();

#endif /* PRINT_H */