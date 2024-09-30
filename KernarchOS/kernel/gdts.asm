; GDT segment selectors
KERNEL_CODE_SEG equ 0x08
KERNEL_DATA_SEG equ 0x10
USER_CODE_SEG equ 0x18 | 3  ; RPL 3
USER_DATA_SEG equ 0x20 | 3  ; RPL 3
TSS_SEG equ 0x28

; Export symbols for use in C code
global gdt_flush
global gdt_entries

section .data
gdt_entries:
    times 6*8 db 0  ; Reserve space for 6 GDT entries (8 bytes each)

section .text
; Function to load the GDT
gdt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the GDTPointer structure
    lgdt [eax]          ; Load the new GDT pointer
    mov ax, KERNEL_DATA_SEG  ; Update the segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp KERNEL_CODE_SEG:.flush  ; Far jump to update CS
.flush:
    ret