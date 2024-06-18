[bits 32]
extern kernel_main
extern gdt_descriptor
extern CODE_SEG
extern DATA_SEG

;multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; magic number
CHECKSUM equ -(MAGIC + MBFLAGS) ; checksum

section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM

section .text

global _start:function (_start.end - _start)

_start:
    ; Initialize base pointer (ebp) to top of the stack
    mov ebp, stack_top
    mov esp, stack_top
    
    call init32

    ; Call kernel_main
    call kernel_main
_start.end:
    hlt

    ; Infinite loop to halt the system
.hang:
    hlt
    jmp .hang

init32:
    ; Load the GDT
    lgdt [gdt_descriptor]

    ; Update the segment registers
    jmp CODE_SEG:.flush

.flush:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

.print:
    ; Set up video memory for message output
    mov esi, hello32
    mov bx, 0x01  ; line
    mov ch, GREEN_ON_BLACK
    call print_string32
    ret

clear_screen:
    .init:
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
        ret

print_string32:
    ; Print the string
    mov edi, VIDEO_MEMORY
    movzx eax, bx       ; Extend bx to 32 bits
    imul eax, 80        ; Multiply by 80
    shl eax, 1          ; Multiply by 2 (equivalent to eax = eax * 2)
    add edi, eax        ; Add to edi (offset to video memory)

.loop32:
    mov al, [esi]       ; Load character from message
    cmp al, 0           ; Check for end of string
    je .done32          ; Exit loop if end of string
    mov ah, ch          ; Set color attribute
    mov [edi], ax       ; Write character and attribute to video memory
    add esi, 1          ; Move to next character
    add edi, 2          ; Move to next cell in video memory
    jmp .loop32         ; Repeat until end of string

.done32:
    ret

; GDT
%include "kernel/gdt.asm"

; Constants and data
hello32 db 'Hello from Protected Mode!', 0
VIDEO_MEMORY equ 0xB8000
WHITE_ON_BLACK equ 0x0F
GREEN_ON_BLACK equ 0x02
SCREEN_SIZE equ 4000 ; 80 columns * 25 rows * 2 bytes per cell

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KiB stack
stack_top:
