.PHONY: all clean
all: kernel.img

run: kernel.img
	qemu-system-x86_64 -bios ../seabios/out/bios.bin -hda kernel.img -display gtk -vnc :${RFB_PORT}

kernel.img: kernel.asm
	nasm -f bin -o kernel.img kernel.asm

clean:
	rm -f kernel.img
