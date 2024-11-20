[ORG 0x7C00]
[BITS 16]

;; Bootloader code

	; Set DS, GS, ES, FS to 0x0
	xor ax, ax
	mov ds, ax
	mov gs, ax
	mov es, ax
	mov fs, ax

	; Stop interrupts. Set SS to 0x8000
	cli
	mov sp, ax
	mov ax, 0x8000
	mov ss, ax
	sti

	push dword 0x0
	push dword 0x1
	push word 0x1000
	push word 0x0000
	push word 0x2
	call read_sectors
	add sp, 0x10

	; Set DS to 0x1000. Far jump to 0x1000:0x0000
	mov ax, 0x1000
	mov ds, ax
	jmp 0x1000:0x0000

bootloader_end:
	jmp bootloader_end

;; Subroutines

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

my_string:
	db 'String', 0x0

disk_address_packet:
	db 0x10
	db 0x00
	dw 0x00 ; Sectors to read
	dw 0x00 ; Dest. offset
	dw 0x00 ; Dest. segment
	dd 0x00 ; Low 4 bytes LBA start
	dd 0x00 ; High 4 bytes LBA start

;; Fill up to 510 bytes. Add 0x55AA boot signature
times 510 - ($ - $$) db 0x0
db 0x55, 0xAA