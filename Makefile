.PHONY: all run clean kernel clean_kernel
all: kernel

run:
	qemu-system-x86_64 -bios seabios/out/bios.bin -fda kernel/kernel.img

kernel:
	cd kernel && $(MAKE)
clean_kernel:
	cd kernel && $(MAKE) clean

clean: clean_kernel
