[ORG 0x0000]
[BITS 16]

;; Second Stage Bootloader Code

	; Read kernel
	push dword 0x0
	push dword 0x3
	push word 0x2000
	push word 0x0000
	push word 0x40
	call read_sectors
	add sp, 0x10

	; Get memory map
	call get_memory_map

	; Load GDTR register
	call load_gdtr

	; Enable protected mode
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax

	; Far jump to kernel code at 0x2000:0x0000 or 0x20000
	jmp dword 0x8:0x20000

ss_bootloader_end:
	jmp ss_bootloader_end	

;; Subroutines

; Get memory map
; Params: NULL
; Return: NULL
get_memory_map:
	
	push bp
	mov bp, sp
	pushfd
	push eax
	push ebx
	push ecx
	push edx
	push es
	push ds
	push di
	push si

	xor ebx, ebx

	get_memory_map_continue:
	mov ax, 0x1000
	mov es, ax 						 	; Set ES to buffer location
	mov di, address_range_descriptor 	; Set DI to buffer location
	mov eax, 0x0000E820 				; Get system memory map
	mov edx, 0x534D4150 				; Set SMAP
	mov ecx, 0x00000018 				; Set size of buffer for result
	int 0x15

	cmp ebx, 0x0
	je get_memory_map_exit

	cld 								; Clear DF - Increment on rep movsb
	mov ecx, 0x00000018					; Set amount of bytes to move

	mov ax, 0x6000						; Set destination to 0x6000:address_range_descriptor_counter
	mov es, ax
	mov ax, [address_range_descriptor_counter]
	mov di, ax

	mov ax, address_range_descriptor  	; Set source to 0x0000:address_range_descriptor
	mov si, ax
	mov ax, 0x1000
	mov ds, ax

	rep movsb

	mov ax, [address_range_descriptor_counter] ; Increment address_range_descriptor_counter
	add ax, 0x18
	mov [address_range_descriptor_counter], ax

	jmp get_memory_map_continue

	get_memory_map_exit:

	mov ax, 0x6000							   ; Save memory map length 
	mov es, ax
	mov ax, 0x0
	mov di, ax
	mov ax, [address_range_descriptor_counter]
	mov es:di, ax

	pop si
	pop di
	pop ds
	pop es
	pop edx
	pop ecx
	pop ebx
	pop eax
	popfd
	mov sp, bp
	pop bp
	ret
; ==

; Load GDTR
; Params: NULL
; Return: NULL
load_gdtr:

	push bp
	mov bp, sp
	push eax
	push ebx

	cli

	xor eax, eax
	xor ebx, ebx
	mov ax, ds
	shl eax, 0x4
	add eax, gdt_table
	mov [gdtr + 0x2], eax
	mov eax, gdt_table_end
	mov ebx, gdt_table
	sub eax, ebx
	mov [gdtr], ax
	lgdt [gdtr]

	pop ebx
	pop eax
	mov sp, bp
	pop bp
	ret
; ==

; Test A20 enabled
; Params: NULL
; Return: AX = 0 if enabled. AX = 1 if disabled
test_a20_enabled:
	
	push bp
	mov bp, sp
	push es
	push ds
	push si
	push di
	push bx
	pushfd

	xor ax, ax
	mov ds, ax
	mov ax, 0x7DFE
	mov si, ax

	xor ax, ax
	not ax
	mov es, ax
	mov ax, 0x7E0E
	mov di, ax

	mov ax, [ds:si]
	mov bx, [es:di]
	cmp ax, bx
	jne test_a20_enabled_true

	mov ax, [ds:si]
	mov bh, al
	mov bl, ah
	mov [ds:si], bx
	mov ax, [es:di]
	cmp ax, bx
	jne test_a20_enabled_true

	mov ax, 0x1
	jmp test_a20_enabled_exit

	test_a20_enabled_true:
	mov ax, 0x0

	test_a20_enabled_exit:

	popfd
	pop bx
	pop di
	pop si
	pop ds
	pop es
	mov sp, bp
	pop bp
	ret
; ==

; Read sectors
; Params: (BP+4) - Sectors to read
; 		  (BP+6) - Dest. offset	
; 		  (BP+8) - Dest. segment
; 		  (BP+A) - Low 4 bytes of LBA start	
; 		  (BP+E) - High 4 bytes of LBA start
read_sectors:
	
	push bp
	mov bp, sp
	push ax
	push si
	push cx
	pushfd

	mov ax, [bp + 0x4]
	mov [disk_address_packet + 0x2], ax
	mov ax, [bp + 0x6]
	mov [disk_address_packet + 0x4], ax
	mov ax, [bp + 0x8]
	mov [disk_address_packet + 0x6], ax
	mov eax, [bp + 0xA]
	mov [disk_address_packet + 0x8], eax
	mov eax, [bp + 0xE]
	mov [disk_address_packet + 0xC], eax

	mov ah, 0x42				; Set extended read
	mov si, disk_address_packet ; Set disk address packet offset
	int 0x13

	popfd
	pop cx
	pop si
	pop ax
	mov sp, bp
	pop bp
	ret
; ==

; Print character
; Params: (BP+4) - Character to print
print_char:

	push bp
	mov bp, sp
	push ax
	push bx

	mov byte al, [bp + 0x4] ; Set character
	mov byte ah, 0x0E		; Set teletype print
	mov byte bl, 0x00  		; Set page number
	int 0x10

	pop bx
	pop ax
	mov sp, bp
	pop bp
	ret
; ==

; Print newline
; Params: NULL
; Return: NULL
print_newline:

	push bp
	mov bp, sp

	push word 0xD
	call print_char
	add sp, 0x2

	push word 0xA
	call print_char
	add sp, 0x2

	mov sp, bp
	pop bp
	ret
; ==

; Print byte
; Params: (BP+4) - Byte to print
print_byte:

	push bp
	mov bp, sp
	push ax

	mov ax, [bp + 0x4]
	shr al, 0x04
	and al, 0x0F
	cmp al, 0x09
	jg print_byte_letter_1
	add al, 0x30
	jmp print_byte_print_1
	print_byte_letter_1:
	add al, 0x37
	print_byte_print_1:
	
	push word ax
	call print_char
	add sp, 0x2

	mov ax, [bp + 0x4]
	and al, 0x0F
	cmp al, 0x09
	jg print_byte_letter_2
	add al, 0x30
	jmp print_byte_print_2
	print_byte_letter_2:
	add al, 0x37
	print_byte_print_2:

	push word ax
	call print_char
	add sp, 0x2
	
	pop ax
	mov sp, bp
	pop bp
	ret
; ==

; Print word
; Params (BP+4) - Word to print
print_word:

	push bp
	mov bp, sp
	push ax

	mov ax, [bp + 0x4]
	shr ax, 0x08

	push word ax
	call print_byte
	add sp, 0x2

	mov ax, [bp + 0x4]
	
	push word ax
	call print_byte
	add sp, 0x2

	pop ax
	mov sp, bp
	pop bp
	ret
; ==

; Print dword
; Params (BP+4) - Dword to print
print_dword:

	push bp
	mov bp, sp
	push ax

	mov eax, [bp + 0x4]
	shr eax, 0x10

	push word ax
	call print_word
	add sp, 0x2

	mov eax, [bp + 0x4]

	push word ax
	call print_word
	add sp, 0x2

	pop ax
	mov sp, bp
	pop bp
	ret
; ==

;; Data structures, string and variables

address_range_descriptor:
	dd 0x0 ; Low 4 bytes base address
	dd 0x0 ; High 4 bytes base address 
	dd 0x0 ; Low 4 bytes length in bytes
	dd 0x0 ; High 4 bytes length in bytes
	dd 0x0 ; Type
	dd 0x0 ; Extended attributes

address_range_descriptor_counter:
	dw 0x2

disk_address_packet:
	db 0x10
	db 0x00
	dw 0x00 ; Sectors to read
	dw 0x00 ; Dest. offset
	dw 0x00 ; Dest. segment
	dd 0x00 ; Low 4 bytes LBA start
	dd 0x00 ; High 4 bytes LBA start

gdtr:
	dw 0x0 ; GDT limit
	dd 0x0 ; GDT base

gdt_table:
	; Null descriptor
	dd 0x0
	dd 0x0
	; Code segment
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 0x9A
	db 0xCF
	db 0x0
	; Data segment
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 0x92
	db 0xCF
	db 0x0
gdt_table_end:

;; Fill up to multiple of 512 bytes
times 512 - (($ - $$) % 512) db 0x0