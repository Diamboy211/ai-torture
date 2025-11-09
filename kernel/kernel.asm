[bits 16]
[org 0x7C00]

start:
	cli
	cld
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00
	mov si, 0x1000
	mov fs, si
	mov [si], dl
	mov al, 0x03
	int 0x10
	inc ah
	mov cx, 0x2706
	int 0x10
	mov ax, 0x0202
	mov cx, 0x0002
	xor dh, dh
	mov bx, 0x7E00
	mov si, bx
	int 0x13
	jnc print
	mov si, disk_err
print:
	mov ax, 0xB800
	mov es, ax
	xor di, di
print1:
	lodsb
	test al, al
	jz hex
	cmp al, 0x0A
	jne print2
	mov ax, di
	mov bx, 160
	div byte bl
	sub bl, ah
	add di, bx
	jmp print1
print2:
	mov ah, 0x07
	stosw
	jmp print1

hex:
	xor si, si
loop:
	cmp di, 80*24*2
	jl loop0
	push si
	xor di, di
	mov si, 160
	push es
	pop ds
	mov cx, 80*24*2
	rep movsb
	sub di, 160
	pop si
	mov ds, cx
loop0:
	xor ax, ax
	int 0x16
	cmp al, 0x0D
	je run
	mov dl, al
	mov ah, 0x07
	stosw
	cmp al, 0x20
	je loop
	xor ax, ax
	int 0x16
	mov ah, 0x07
	stosw

	cmp dl, 0x40
	jb loop1
	add dl, 9
loop1:
	cmp al, 0x40
	jb loop2
	add al, 9
loop2:
	mov ah, dl
	shl al, 4
	shr ax, 4
	mov [fs:si], al
	inc si
	jmp loop
run:
	jmp 0x1000:0

disk_err:
	db "Reading sectors 0/0/2 - 0/0/3 failed.", 0x0A, 0x00

times (510-($-$$)) db 0
dw 0xAA55
db "---SECTORS 0/0/2 - 0/0/3---", 0x0A
db "These sectors are for persistent documentation. Modify as needed.", 0x0A
db "Sector 0/0/1: The boot sector saves the boot drive number to 0:0x1000, "
db "sets the video mode to 03h, "
db "prints the content of sectors 0/0/2 - 0/0/3 as an ASCIIZ string to the screen, "
db "then waits for keyboard input.", 0x0A
db "When hex digits (0-9,A-F,a-f) are typed, they are echoed on the screen.", 0x0A
db "Every pair of hex digits typed will be converted into a byte as if they were a hex number, "
db "then saved sequentially to a buffer starting at 0x1000:0.", 0x0A
db "When the Enter key is pressed and an even number of hex digits has been typed, "
db "the program will execute a far jump to 0x1000:0.", 0x0A
db "Sector 0/0/2 - 0/0/3: This document. Modify as needed.", 0x0A
db "Sector 0/0/4 - 79/1/18: Free to use.", 0x0A
db 0
times (1536-($-$$)) db 0
times (512*18*80*2-($-$$)) db 0
