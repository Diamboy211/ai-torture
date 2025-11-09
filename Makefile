.PHONY: all run_qemu run_jail clean seabios liboai kernel clean_seabios clean_liboai clean_kernel

export RFB_PORT ?= 1

all: seabios liboai kernel jail

run_qemu: seabios kernel
	$(MAKE) -C kernel run
run_jail: jail
	$(MAKE) -C jail run

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
