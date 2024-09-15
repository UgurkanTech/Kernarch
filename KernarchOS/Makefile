# Variables
AS=nasm
LD=ld
CC=gcc
CFLAGS=-m32 -ffreestanding -O1 -Wall -Wextra -MMD -c -g -fno-exceptions -fno-rtti -std=c++17
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
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile C files
%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Rule to assemble .asm files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Rule to make the kernel
$(KERNEL): $(COBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(COBJECTS)

# Rule to make the ISO
$(ISO): $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.bin
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $@ iso

# Rule to clean the build
clean:
	rm -f $(KERNEL) $(ISO) $(COBJECTS) $(ASM_OBJECTS) $(DEPS)
	rm -rf iso

# Include dependencies
-include $(DEPS)

.PHONY: all clean
