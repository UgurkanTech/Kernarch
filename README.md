# Kernarch - created by Uğurkan Hoşgör

This is a work-in-progress operating system Micro-kernel being developed with a specific purpose in mind. As part of an ongoing journey in OS development, it's a creative playground where new concepts and skills are discovered.

To streamline the build process, Docker is used to compile the kernel and create .iso images. This approach ensures a consistent and isolated environment, making the development process more efficient and reproducible.

![demo](https://github.com/user-attachments/assets/c8c93420-0236-41e7-a0ce-25666e9afe6e)

## Current Features

Project Status: [■■■■■■■■■■■■---------------------] 60% Complete


| **Feature**                              | **Status**          |
|------------------------------------------|---------------------|
| GRUB Bootloader                          | ✔️   |
| 32-bit Protected Mode                    | ✔️   |
| Global Descriptor Table (GDT)            | ✔️   |
| C Runtime Integration                    | ✔️   |
| Basic Memory Management                  | ✔️   |
| Interrupt Handling                       | ✔️   |
| VGA Display Features                     | ✔️   |
| Keyboard Handling                        | ✔️   |
| Shell Implementation                     | ✔️   |
| PIC (Programmable Interrupt Controller)  | ✔️   |
| IDT (Interrupt Descriptor Table)         | ✔️   |
| ISRs (Interrupt Service Routines)        | ✔️   |
| Paging                                   | ✔️   |
| User Mode                                | ✔️   |
| Process Scheduling                       | ✔️   |
| System Calls                             | ✔️   |
| Multitasking Support                     | ✔️   |
| C Standard Library                       | ❌   |
| VirtIO Network Driver                    | ❌   |
| VirtIO Block Driver                      | ❌   |
| IDE Drivers                              | ❌   |
| Graphical User Interface (GUI)           | ❌   |



- ✔️ = Completed  
- 🟡 = In Progress (Implemented but not functional)  
- ❌ = Not Implemented (Upcoming features)

This adds clarity and a visual distinction to the project's status for users.

## Building ISO with Docker and testing with QEMU

The ISO is built using Docker and upon successful completion, the ISO file will be created in the output directory. To initiate the process, the start.sh script is run.

```bash
./build.sh
```

Once the ISO is built, it will be tested using QEMU. Please ensure QEMU is installed on your host machine. 


## Testing with Qemu

To test purposes, use QEMU. Run the assembled kernel binary with the following command:

```bash
qemu-system-x86_64 -m 1G -netdev user,id=mynet0 -device rtl8139,netdev=mynet0 -cdrom KernarchOS.iso -drive file=disk.img,format=raw,if=ide,index=0
```

## Creating an Empty Disk Image

To create a 512MB empty hard disk image, use the following command:

```bash
qemu-img create -f raw disk.img 512M 
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
