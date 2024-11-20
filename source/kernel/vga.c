#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MISC_OUTPUT_REGISTER_READ 						0x3CC
#define MISC_OUTPUT_REGISTER_WRITE 						0x3C2
#define FEATURE_CONTROL_REGISTER_READ					0x3CA
#define FEATURE_CONTROL_REGISTER_MONO_WRITE				0x3BA
#define FEATURE_CONTROL_REGISTER_COLOR_WRITE			0x3DA
#define INPUT_STATUS_0_REGISTER_READ					0x3C2
#define INPUT_STATUS_1_REGISTER_MONO_READ				0x3BA
#define INPUT_STATUS_1_REGISTER_COLOR_READ				0x3DA

#define GRAPHICS_ADDRESS_REGISTER						0x3CE
#define GRAPHICS_DATA_REGISTER							0x3CF

#define GRAPHICS_SET_RESET_REGISTER_INDEX				0x00
#define GRAPHICS_ENABLE_SET_RESET_REGISTER_INDEX		0x01
#define GRAPHICS_COLOR_COMPARE_REGISTER_INDEX			0x02
#define GRAPHICS_DATA_ROTATE_REGISTER_INDEX				0x03
#define GRAPHICS_READ_MAP_SELECT_REGISTER_INDEX			0x04
#define GRAPHICS_GRAPHICS_MODE_REGISTER_INDEX			0x05
#define GRAPHICS_MISC_GRAPHICS_REGISTER_INDEX			0x06
#define GRAPHICS_COLOR_DONT_CARE_REGISTER_INDEX			0x07
#define GRAPHICS_BIT_MASK_REGISTER_INDEX				0x08

#include "time.h"
#include "io.h"
#include "print.h"

void vga_init() {

	uint8_t misc_output = ioreadb(MISC_OUTPUT_REGISTER_READ);
	sleep(1);

	print_string("MISC OUTPUT register: 0x");
	print_byte(misc_output);
	print_newline();

	uint8_t graphics_address = ioreadb(GRAPHICS_ADDRESS_REGISTER);
	sleep(1);

	print_string("GRAPHICS ADDRESS register: 0x");
	print_byte(graphics_address);
	print_newline();

	uint8_t graphics_mode = ioreadb(GRAPHICS_DATA_REGISTER);
	sleep(1);	

	print_string("GRAPHICS MODE register: 0x");
	print_byte(graphics_mode);
	print_newline();

	return;

}