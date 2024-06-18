# Kernarch - created by Uğurkan Hoşgör

This is a work-in-progress operating system kernel being developed with a specific purpose in mind. As part of an ongoing journey in OS development, it's a creative playground where new concepts and skills are discovered.

<img width="456" alt="Screenshot at Jun 16 19-41-07" src="https://github.com/UgurkanTech/Kernarch/assets/39236929/0db862e3-89a0-43b3-bb7d-c9b6362714a5">

## Current Features

- GRUB bootloader
- 32-bit protected mode
- Global Descriptor Table (GDT)
- C runtime Integration


## Upcoming Features - WIP

- Basic Memory Management
- Interrupt Handling
- VGA Display Features
- File System Support
- Device Drivers
- Graphical User Interface (GUI)
- Networking Capabilities
- Multitasking Support

## Building ISO with Docker and testing with QEMU

The ISO is built using Docker and upon successful completion, the ISO file will be created in the output directory. To initiate the process, the start.sh script is run.

```bash
./build.sh
```

Once the ISO is built, it will be tested using QEMU. Please ensure QEMU is installed on your host machine. 


## Testing with Qemu

To test purposes, use QEMU. Run the assembled kernel binary with the following command:

```bash
qemu-system-x86_64 -cdrom output/KernarchOS.iso
```

## Manuel building and linking

Manual building and linking is not recommended. However, if needed, the assembly can be done with NASM and the compilation with GCC as shown below:

```bash
# Assemble with NASM
nasm -f elf32 boot.asm -o boot.o

# Compile with GCC
gcc -m32 -c kernel.cpp -o kernel.o

# Link with ld
ld -m elf_i386 -T link.ld -o kernel.bin boot.o kernel.o
```
