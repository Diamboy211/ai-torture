.PHONY: all clean
all: kernel.img

kernel.img: kernel.asm
	nasm -f bin -o kernel.img kernel.asm

clean:
	rm -f kernel.img
