; Macro to create ISR stubs
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0                 ; push dummy error code
    push %1                ; push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1                ; push interrupt number
    jmp isr_common_stub
%endmacro

; Create ISR stubs
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Define IRQ handlers
%assign i 32
%rep 224
ISR_NOERRCODE i
%assign i i+1
%endrep


extern isr_handler

; Common ISR stub
isr_common_stub:
    pusha                  ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds
    push eax               ; Save the data segment descriptor

    mov ax, 0x10           ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp               ; Push a pointer to the interrupt frame
    call isr_handler
    add esp, 4             ; Remove the pushed parameter

    pop eax                ; Reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                   ; Pops edi,esi,ebp,esp,ebx,edx,ecx,eax
    add esp, 8             ; Cleans up the pushed error code and pushed ISR number
    iret                   ; Pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dd isr%+i
%assign i i+1
%endrep