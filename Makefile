# Makefile

all:
	$(MAKE) -C KernarchOS

kernel:
	$(MAKE) -C KernarchOS kernel

iso:
	$(MAKE) -C KernarchOS iso

run:
	$(MAKE) -C KernarchOS run

clean:
	$(MAKE) -C KernarchOS clean

.PHONY: all kernel iso run clean