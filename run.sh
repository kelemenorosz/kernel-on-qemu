#!/bin/bash

python3 elf_reader.py

while read suffix; do
cat "${suffix}";
done < cat_order > bootloader

python3 make_large.py

qemu-system-i386 -cpu core2duo,-syscall,-lm -m 1g -drive format=raw,id=disk,file=bootloader_large,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 \
-netdev tap,id=u1,ifname=tap0,script=no,downscript=no -device e1000,netdev=u1 \
-object filter-dump,id=f1,netdev=u1,file=qemu_ethernet_dump.dat \
-serial file:serial.log \
-monitor stdio

#-S -s \
#qemu-system-i386 -drive id=disk,file=bootloader_large,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -boot menu=on
#-netdev user,id=u1 -device e1000,netdev=u1 \

