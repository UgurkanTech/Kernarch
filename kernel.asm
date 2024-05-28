[bits 16]
[org 0x7c00]

section .text
    global _start

_start:
    xor ax, ax          ; AX = 0
    mov ds, ax          ; DS = 0
    mov es, ax          ; ES = 0
    mov ss, ax          ; SS = 0
    mov sp, 0x7c00      ; Set stack pointer to 0x7c00 (top of stack)
    
    mov si, hello
    call print_string
    
    call switch_16_to_32
    
    jmp $

print_string:
    mov ah, 0Eh
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

switch_16_to_32:
    cli                 ; Disable interrupts
    lgdt [gdt_descriptor] ; Load the GDT descriptor

    ; Enable the A20 line
    in al, 0x92
    or al, 00000010b    ; Set bit 1 (A20 enable)
    out 0x92, al

    ; Set PE (Protection Enable) bit in CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the prefetch queue and switch to 32-bit mode
    jmp CODE_SEG:init32

[bits 32]
init32:
    ; Set up data segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Now in 32-bit mode
    ; Print message in protected mode to verify switch
    mov esi, hello32
    call print_string32

    ; Infinite loop to halt the system
    hlt

print_string32:
    mov edx, esi
.loop32:
    mov al, [edx]
    cmp al, 0
    je .done32
    mov ah, 0x0E
    int 0x10
    inc edx
    jmp .loop32
.done32:
    ret

; GDT
%include "gdt.asm"

hello db 'Hello world!', 0x0a, 0
hello32 db 'Hello from Protected Mode!', 0x0a, 0

times 510-($-$$) db 0
dw 0xaa55
