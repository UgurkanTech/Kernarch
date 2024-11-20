[bits 32]
extern kernel_main
extern stack_top

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
    ; Initialize base pointer (ebp) and stack pointer (esp)
    mov ebp, stack_top
    mov esp, stack_top

    ;Enable FPU
    mov edx, cr0 
    and edx, 0xFFFFFFFB ; Clear EM bit (bit 2) 
    mov cr0, edx

    ;Enable SSE
    mov edx, cr4
    or edx, 0x00000200    ; Set OSFXSR bit (bit 9)
    or edx, 0x00000400    ; Set OSXMMEXCPT bit (bit 10)
    mov cr4, edx
    
    ;Initialize FPU
    fninit;

    push eax
    push ebx
    
    ; Call kernel_main
    call kernel_main
_start.end:
    ; Infinite loop to halt the system
.hang:
    hlt
    jmp .hang