# KernArch

KernArch is a work-in-progress operating system kernel with a specific purpose. It's currently being built with NASM and tested with QEMU.

## Current Features

- 32-bit protected mode
- Global Descriptor Table (GDT)

## Upcoming Features

- Interrupts
- C runtime
- VGA features
- File systems
- Devices
- GUI

## Building

Currently, the project is assembled using NASM. Use the following command to assemble the kernel:

```bash
nasm -f bin -o kernel.bin kernel.asm 
```

## Testing with Qemu

To test the project, use QEMU. Run the assembled kernel binary with the following command:

```bash
qemu-system-x86_64 -serial stdio -drive format=raw,file=kernel.bin
```

## TODO - Building with C lang

In the future, the project will be compiled with GCC and linked together to create the final kernel binary. The commands for this are not yet finalized, but it will involve steps similar to the following:

```bash
# Assemble with NASM
nasm -f elf32 boot.asm -o boot.o

# Compile with GCC
gcc -m32 -c kernel.c -o kernel.o

# Link with ld
ld -m elf_i386 -T link.ld -o kernel.bin boot.o kernel.o
```