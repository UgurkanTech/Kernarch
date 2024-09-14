# Kernarch - created by Uğurkan Hoşgör

This is a work-in-progress operating system kernel being developed with a specific purpose in mind. As part of an ongoing journey in OS development, it's a creative playground where new concepts and skills are discovered.

![image](https://github.com/user-attachments/assets/2c76ec33-cc93-4619-9040-22a9cd75cced)



## Current Features

Project Status: [■■■■■■■■■■■■--------------------] 35% Complete

- GRUB bootloader
- 32-bit protected mode
- Global Descriptor Table (GDT)
- C runtime Integration
- Basic Memory Management
- Interrupt Handling
- VGA Display Features
- Keyboard Handling
- Shell Implementation
- Paging
- PIC (Programmable Interrupt Controller)
- IDT (Interrupt Descriptor Table)
- ISRs (Interrupt Service Routines)


## Upcoming Features - WIP

- File System Support
- Device Drivers
- Graphical User Interface (GUI)
- Networking Capabilities
- Multitasking Support
- Process Scheduling
- System Calls
- User Mode

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
