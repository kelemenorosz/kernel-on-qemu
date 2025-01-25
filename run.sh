#!/bin/bash

python3 elf_reader.py

while read suffix; do
cat "${suffix}";
done < cat_order > bootloader

python3 make_large.py

qemu-system-i386 -cpu core2duo,-syscall,-lm -m 1g -drive format=raw,id=disk,file=bootloader_large,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 \
-serial file:serial.log \
-monitor stdio

# -S -s \
# -device isa-vga \
# qemu-system-i386 -drive id=disk,file=bootloader_large,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -boot menu=on
# -netdev user,id=u1 -device e1000,netdev=u1 \

