.PHONY: all run clean seabios liboai kernel clean_seabios clean_liboai clean_kernel
all: seabios liboai kernel jail

run:
	qemu-system-x86_64 -bios seabios/out/bios.bin -fda kernel/kernel.img

seabios:
	$(MAKE) -C seabios
liboai:
	mkdir -p build_liboai
	cmake -S liboai -B build_liboai --install-prefix ${PWD}/build_liboai
	$(MAKE) -C build_liboai
	$(MAKE) -C build_liboai install
kernel:
	$(MAKE) -C kernel
jail: liboai
	$(MAKE) -C jail

clean_liboai:
	rm -rf build_liboai/*
clean_seabios:
	$(MAKE) -C seabios clean
clean_kernel:
	$(MAKE) -C kernel clean
clean_jail:
	$(MAKE) -C jail clean

clean: clean_kernel clean_liboai clean_seabios clean_jail
