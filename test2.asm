[bits 16] ; Switch to 16-bit mode for bootloader compatibility
[org 0x7c00] ; Set the start address

section .text

; Bootloader code (in 16-bit mode)
init:
    ; Initialize the kernel here (if needed)
    jmp switch_to_64   ; Jump to switch_to_64 label to switch to 64-bit mode

switch_to_64:
    cli                ; Clear interrupts to prevent any interruptions during mode switch
    mov eax, cr0       ; Move the value of CR0 register into EAX
    or eax, 0x1        ; Set the first bit of CR0 register to 1 (to enable protected mode)
    mov cr0, eax       ; Move the modified value of EAX back to CR0 register
    jmp CODE_SEG:init64 ; Jump to 64-bit code segment with long jump
    hlt                ; Stop execution (This will never execute because we're jumping to a different code segment)

section .text64

init64:
    ; Initialize the kernel in 64-bit mode (if needed)
    ; Start execution from an empty function or any other necessary code
    jmp $

; Empty function as a starting point
empty_function:
    ret

times 510-($-$$) db 0 ; Fill the output file with zeroes until 510 bytes are full
dw 0xaa55 ; Magic number that tells the BIOS this is bootable
