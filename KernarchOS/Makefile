# Variables
AS=nasm
LD=ld
CC=gcc
CFLAGS=-m32 -w -ffreestanding -nostdlib -msse -O1 -Wall -Wextra -MMD -c -g -fno-exceptions  -Wno-unused-variable -Wno-unused-parameter -fno-rtti -std=c++17
LDFLAGS=-T linker.ld -nostdlib -m elf_i386
ASFLAGS=-felf32
KERNEL=kernel.bin
ISO=kernarch.iso
ASM_SOURCES=$(wildcard kernel/*.asm)
CSOURCES=$(wildcard kernel/*.cpp)
COBJECTS=$(CSOURCES:.cpp=.o)
DEPS=$(COBJECTS:.o=.d)
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)

# Default make target
all: $(KERNEL) $(ISO)

# Rule to compile C++ files
%.o: %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@ 

# Rule to compile C files
%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule to assemble .asm files
%.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

# Rule to make the kernel
$(KERNEL): $(COBJECTS) $(ASM_OBJECTS)
	@$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(COBJECTS) 

# Rule to make the ISO
$(ISO): $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.bin
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $@ iso


# Rule to run QEMU
run: $(ISO)
	cd output && \
	disk_image=disk.img && \
	if [ -f "$$disk_image" ]; then \
		echo "The file $$disk_image exists."; \
	else \
		echo "The file $$disk_image does not exist. Creating it now..."; \
		qemu-img create -f raw $$disk_image 512M; \
		echo "File $$disk_image created."; \
	fi && \
	qemu-system-i386 -display default,show-cursor=on -m 1G -netdev user,id=mynet0 -device rtl8139,netdev=mynet0 -cdrom $(ISO) -drive file=$$disk_image,format=raw,if=ide,index=0

# Rule to clean the build
clean:
	rm -f $(KERNEL) $(ISO) $(COBJECTS) $(ASM_OBJECTS) $(DEPS)
	rm -rf iso

# Include dependencies
-include $(DEPS)

.PHONY: all clean
