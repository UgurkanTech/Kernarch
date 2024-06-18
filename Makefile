# Makefile

all:
	$(MAKE) -C KernarchOS

run:
	$(MAKE) -C KernarchOS run

iso:
	$(MAKE) -C KernarchOS iso

runiso:
	$(MAKE) -C KernarchOS runiso

clean:
	$(MAKE) -C KernarchOS clean

.PHONY: all run iso runiso clean