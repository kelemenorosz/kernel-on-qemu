#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "vesa_vbe.h"
#include "virtual8086.h"
#include "memory.h"
#include "print.h"
#include "serial.h"

typedef struct __attribute__((__packed__)) VESA_VBE_SUPERVGA_INFO_2_0 { 

	uint8_t signature[4];
	uint16_t version;
	uint32_t OEM_name_ptr;
	uint32_t capabilities;
	uint32_t video_mode_list_ptr;
	uint16_t total_video_memory_64k;
	uint16_t OEM_software_version;
	uint32_t vendor_name_ptr;
	uint32_t product_name_ptr;
	uint32_t product_revision_string_ptr;
	uint16_t vbe_af_version;
	uint32_t accelerated_video_mode_list_ptr;

} VESA_VBE_SUPERVGA_INFO_2_0;

typedef struct __attribute__((__packed__)) VESA_VBE_SUPERVGA_MODE_INFO { 

	uint16_t mode_attr;
	uint8_t window_attrA;
	uint8_t window_attrB;
	uint16_t window_granularity;
	uint16_t window_size;
	uint16_t windowA_start_segment;
	uint16_t windowB_start_segment;
	uint32_t window_function_ptr;
	uint16_t bytes_per_scanline;
	uint16_t width;
	uint16_t height;
	uint8_t char_width;
	uint8_t char_height;
	uint8_t mem_planes;
	uint8_t bpp;
	uint8_t banks;
	uint8_t mem_model_type;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved;
	uint8_t red_mask_size;
	uint8_t red_field_pos;
	uint8_t green_mask_size;
	uint8_t green_field_size;
	uint8_t blue_mask_size;
	uint8_t blue_field_size;
	uint8_t reserved_mask_size;
	uint8_t reserved_mask_pos;
	uint8_t direct_color_mode_info;
	uint32_t framebuffer_ptr;
	uint32_t offscreen_mem_ptr;
	uint16_t offscreen_mem_size;

} VESA_VBE_SUPERVGA_MODE_INFO;

#define flat_address(address) \
	((((address & 0xFFFF0000) >> 0x10) << 4) + (address & 0xFFFF))

void vesa_vbe_init() {

	V86_DESC v86_desc = {};

	// -- BIOS INT 0x10; AX 0x4F00

	uint8_t v86_buf[1024];
	VESA_VBE_SUPERVGA_INFO_2_0* v86_buf_s = (VESA_VBE_SUPERVGA_INFO_2_0*)&v86_buf[0];

	v86_buf_s->signature[0] = 0x56;
	v86_buf_s->signature[1] = 0x42;
	v86_buf_s->signature[2] = 0x45;
	v86_buf_s->signature[3] = 0x32;

	memset(&v86_desc, 0, sizeof(V86_DESC));
	
	v86_desc.eax = 0x4F00;
	v86_desc.es = ((uint32_t)&v86_buf[0] & 0xF0000) >> 4;
	v86_desc.edi = (uint32_t)&v86_buf[0] & 0xFFFF;

	virtual8086(&v86_desc);

	// TODO: Check for errors

	v86_buf[4] = 0;
	print_string((char*)&v86_buf[0]);
	print_newline();

	print_string("V86 0x4F00 EAX: ");
	print_dword(v86_desc.eax);
	print_newline();

	uint32_t vendor_name = flat_address(*(uint32_t*)&v86_buf[0x16]);
 
	print_string((char*)vendor_name);
	print_newline();

	uint16_t* video_mode_list = (uint16_t*)flat_address(v86_buf_s->video_mode_list_ptr);
	
	serial_write_string("[VESA_VBE_INIT] Supported video modes: ");
	serial_write_newline();

	while (*video_mode_list != 0xFFFF) {

		serial_write_string("0x");
		serial_write_word(*(video_mode_list));
		serial_write_newline();

		video_mode_list++;

	}

	// -- BIOS INT 0x10; AX = 0x4F01; CX = 0x118

	uint8_t v86_mode_buf[1024];
	VESA_VBE_SUPERVGA_MODE_INFO* v86_mode_buf_s = (VESA_VBE_SUPERVGA_MODE_INFO*)&v86_mode_buf[0];
	memset(v86_mode_buf_s, 0, sizeof(VESA_VBE_SUPERVGA_MODE_INFO));

	memset(&v86_desc, 0, sizeof(V86_DESC));

	v86_desc.eax = 0x4F01;
	v86_desc.ecx = 0x118; 
	v86_desc.es = ((uint32_t)&v86_mode_buf[0] & 0xF0000) >> 4;
	v86_desc.edi = (uint32_t)&v86_mode_buf[0] & 0xFFFF;

	virtual8086(&v86_desc);

	print_string("V86 0x4F01 EAX: ");
	print_dword(v86_desc.eax);
	print_newline();

	print_string("Mode attributes: ");
	print_word(v86_mode_buf_s->mode_attr);
	print_newline();

	print_string("Width: ");
	print_word(v86_mode_buf_s->width);
	print_newline();

	print_string("Height: ");
	print_word(v86_mode_buf_s->height);
	print_newline();

	print_string("Bpp: ");
	print_byte(v86_mode_buf_s->bpp);
	print_newline();

	print_string("Memory model: ");
	print_byte(v86_mode_buf_s->mem_model_type);
	print_newline();

	print_string("Framebuffer_ptr: ");
	print_dword(v86_mode_buf_s->framebuffer_ptr);
	print_newline();

	// -- BIOS INT 0x10; AX = 0x4F02; BX = 0x4118

	memset(&v86_desc, 0, sizeof(V86_DESC));

	v86_desc.eax = 0x4F02;
	v86_desc.ecx = 0x4118; 

	virtual8086(&v86_desc);

	serial_write_string("[VESA_VBE_INIT] AX 0x4F02 return code: 0x");
	serial_write_dword(v86_desc.eax);
	serial_write_newline();

	// -- Pallette

	memset(&v86_desc, 0, sizeof(V86_DESC));

	v86_desc.eax = 0x4F08;
	v86_desc.ebx = 0x1; 

	virtual8086(&v86_desc);

	serial_write_string("[VESA_VBE_INIT] AX 0x4F08 return code: 0x");
	serial_write_dword(v86_desc.eax);
	serial_write_newline();

	uint16_t* linear_framebuffer = (uint16_t*)v86_mode_buf_s->framebuffer_ptr;

	serial_write_string("[VESA_VBE_INIT] Linear framebuffer: ");
	serial_write_dword((uint32_t)linear_framebuffer);
	serial_write_newline();

	for (size_t i = 0; i < 0x100; ++i) {
		linear_framebuffer[0] = i << 4;
	
		linear_framebuffer++;
	}

	return;

}