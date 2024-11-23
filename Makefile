KERNEL_FILE := kernel

BUILD_DIR := ./build
KERNEL_DIR := ./source/kernel
BOOTLOADER_DIR := ./source/boot

NASM := nasm
AS := ~/build_magic/install-dir-cc/bin/i686-elf-as
CC := ~/build_magic/install-dir-cc/bin/i686-elf-gcc

BOOTLOADER_SRC := $(shell find $(BOOTLOADER_DIR) -name '*.asm')
BOOTLOADER_OBJ := $(BOOTLOADER_SRC:%=$(BUILD_DIR)/%.bin)

KERNEL_SRC := $(shell find $(KERNEL_DIR) -name '*.s' -or -name '*.c')
KERNEL_OBJ := $(KERNEL_SRC:%=$(BUILD_DIR)/%.o)

INCLUDE_DIRS := $(shell find $(KERNEL_DIR) -type d)
INCLUDE_FLAGS := $(INCLUDE_DIRS:%=-I%)

all: $(BUILD_DIR)/$(KERNEL_FILE) $(BOOTLOADER_OBJ)
	echo "Done"

$(BUILD_DIR)/$(KERNEL_FILE): $(KERNEL_OBJ)
	$(CC) -T $(KERNEL_DIR)/linker.ld -o $@ -ffreestanding -O2 -nostdlib -g $(BUILD_DIR)/$(KERNEL_DIR)/kernel_start.s.o $(filter-out $(BUILD_DIR)/$(KERNEL_DIR)/kernel_start.s.o,$(KERNEL_OBJ)) -lgcc
	cp $@ ./

$(BUILD_DIR)/$(BOOTLOADER_DIR)%.asm.bin: $(BOOTLOADER_DIR)%.asm
	mkdir -p $(dir $@)
	$(NASM) -f bin -o $@ $<
	cp $@ ./

$(BUILD_DIR)/$(KERNEL_DIR)%.s.o: $(KERNEL_DIR)%.s
	mkdir -p $(dir $@)
	$(AS) $< -o $@

$(BUILD_DIR)/$(KERNEL_DIR)%.c.o: $(KERNEL_DIR)%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(INCLUDE_FLAGS) -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
