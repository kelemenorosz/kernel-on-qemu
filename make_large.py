
def main():
	with open("bootloader", mode='rb') as bootloader:
		with open("bootloader_large", mode='wb') as bootloader_large:
			bootloader_buffer = bootloader.read()
			bootloader_size = len(bootloader_buffer)
			bootloader_large_size = 2 ** 20
			bootloader_large_fill_size = bootloader_large_size - bootloader_size
			bootloader_large.write(bootloader_buffer)
			bootloader_large.write(bytearray('\0' * bootloader_large_fill_size, encoding='utf-8'))

if __name__ == "__main__":
	main()