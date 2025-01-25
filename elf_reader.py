import struct

def main():
	with open("kernel", mode='rb') as kernel_file:
		kernel_file_buf = kernel_file.read()
		kernel_header = struct.unpack("<B3sBBBB8sHHL", kernel_file_buf[:24])
		print(kernel_header)

		bit_version = kernel_header[2]
		kernel_header_ext = None
		if bit_version == 1:
			kernel_header_ext = struct.unpack("<LLLLHHHHHH", kernel_file_buf[24:52])
			print(kernel_header_ext)

		program_header_count = kernel_header_ext[6]
		program_header_offset = kernel_header_ext[1]
		program_header_size = kernel_header_ext[5]

		with open("kernel_extract.bin", mode='wb') as kernel_extract_file:

			base_addr = 0
			write_addr = 0

			for i in range(program_header_count):
				program_header = struct.unpack("<LLLLLLLL", kernel_file_buf[program_header_offset:(program_header_offset + program_header_size)])
				print(program_header)

				current_address = program_header[3]

				if base_addr == 0:
					base_addr = program_header[3]
				if write_addr == 0:
					write_addr = base_addr

				print("base address: " + hex(base_addr))
				print("write address: " + hex(write_addr))
				print("current address: " + hex(program_header[3]))

				if current_address > write_addr:
					kernel_extract_file.write(bytearray('\0' * (current_address - write_addr), encoding='utf-8'))
					write_addr = current_address

				p_filesz = program_header[4]
				p_offset = program_header[1]
				fill_count = (p_filesz + p_offset) % 4096
				fill_count = 4096 - fill_count
				kernel_extract_file.write(kernel_file_buf[p_offset:(p_offset + p_filesz)])
				if p_filesz != 0:
					kernel_extract_file.write(bytearray('\0' * fill_count, encoding='utf-8'))
				program_header_offset += program_header_size

				write_size = p_filesz + fill_count
				print("write size: " + hex(write_size))
				write_addr += write_size

if __name__ == "__main__":
	main()