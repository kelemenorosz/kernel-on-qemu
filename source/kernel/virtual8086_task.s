.org 0x0000		# code runs at 0x3000:0x0000
.code16			# 16-bit code for virtual 8086
.section .virtual8086, "ax", @progbits

start:

	mov $0x0013, %ax
	int $0x10
	mov $0x02, %cx

v86_test_label:

	jmp start
	