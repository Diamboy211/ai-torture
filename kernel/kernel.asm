[bits 16]
[org 0x7C00]

start:
	cli
	xor ax, ax
	mov ds, ax
	mov [0x1000], dl
	mov ah, 0x10
	mov es, ax
	cld
	xor di, di
	xor bx, bx
loop:
	xor ax, ax
	int 0x16
	cmp al, 0x0D
	je run
	mov dl, al
	mov ah, 0x0E
	int 0x10
	cmp dl, 0x20
	je loop
	xor ax, ax
	int 0x16
	mov dh, al
	mov ah, 0x0E
	int 0x10
	cmp dl, 0x40
	jb loop1
	add dl, 9
loop1:
	cmp dh, 0x40
	jb loop2
	add dh, 9
loop2:
	shl dh, 4
	rol dx, 4
	mov al, dl
	stosb
	jmp loop
run:
	jmp 0x1000:0

times (510-($-$$)) db 0
dw 0xAA55
times (512*18*80*2-($-$$)) db 0
