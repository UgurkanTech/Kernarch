[bits 16]
[org 0x7c00]

section .text
    global _start

_start:
    xor ax, ax          ; AX = 0
    mov ds, ax          ; DS = 0
    mov es, ax          ; ES = 0
    mov ss, ax          ; SS = 0
    mov ax, 0x9000  ; Set AX to 0x9000
    mov ss, ax     ; Set stack segment to 0x9000
    mov sp, 0xFFFF ; Set stack pointer to 0xFFFF (top of the stack)
    
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
    .loop:
        jmp .loop

print_string32:
    ; Clear the screen
    mov edi, VIDEO_MEMORY
    mov ecx, SCREEN_SIZE
    mov ax, ' ' | (WHITE_ON_BLACK << 8)
.clear_screen:
    dec ecx
    js .done_clear
    mov [edi], ax
    add edi, 2
    jmp .clear_screen
.done_clear:

    ; Print the string
    mov edi, VIDEO_MEMORY
    mov esi, hello32
.loop32:
    mov al, [esi]
    cmp al, 0
    je .done32
    mov ah, GREEN_ON_BLACK
    mov [edi], ax
    add esi, 1
    add edi, 2
    jmp .loop32
.done32:
    ret

; GDT
%include "gdt.asm"

hello db 'Hello world!', 0x0a, 0
hello32 db 'Hello from Protected Mode!', 0x0a, 0
VIDEO_MEMORY equ 0xB8000
WHITE_ON_BLACK equ 0x0F
GREEN_ON_BLACK equ 0x02
SCREEN_SIZE equ 4000 ; 80 columns * 25 rows * 2 bytes per cell

times 510-($-$$) db 0
dw 0xaa55
