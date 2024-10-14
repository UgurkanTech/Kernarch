; context_switch.asm

section .text
global asm_switch_to_user
global asm_switch_to_kernel
extern current_context

; Constants for segment selectors
USER_DATA_SELECTOR equ 0x23  ; User mode data segment selector
USER_CODE_SELECTOR equ 0x1B  ; User mode code segment selector
KERNEL_DATA_SELECTOR equ 0x10  ; Kernel mode data segment selector
KERNEL_CODE_SELECTOR equ 0x08  ; Kernel mode code segment selector

struc CONTEXT
    .eax: resd 1
    .ebx: resd 1
    .ecx: resd 1
    .edx: resd 1
    .esi: resd 1
    .edi: resd 1
    .ebp: resd 1
    .esp: resd 1
    .eip: resd 1
    .eflags: resd 1
    .cr3: resd 1
endstruc

; Switch from kernel to user mode
asm_switch_to_user:
    ; Assume the argument (pointer to user context) is on the stack
    mov ebp, [esp + 4]  ; Get pointer to user context

    ; Load user context
    mov eax, [ebp + CONTEXT.eax]
    mov ebx, [ebp + CONTEXT.ebx]
    mov ecx, [ebp + CONTEXT.ecx]
    mov edx, [ebp + CONTEXT.edx]
    mov esi, [ebp + CONTEXT.esi]
    mov edi, [ebp + CONTEXT.edi]

    ; Set up stack for iret
    push USER_DATA_SELECTOR     ; SS
    push dword [ebp + CONTEXT.esp]  ; ESP
    push dword [ebp + CONTEXT.eflags]  ; EFLAGS
    push USER_CODE_SELECTOR     ; CS
    push dword [ebp + CONTEXT.eip]  ; EIP

    ; Load CR3 if necessary (for address space switch)
    mov eax, [ebp + CONTEXT.cr3]
    mov cr3, eax

    ; Load user data segment
    mov ax, USER_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore last register
    mov ebp, [ebp + CONTEXT.ebp]

    ; Switch to user mode
    iret

; Switch from user to kernel mode
asm_switch_to_kernel:
    ; This will be called via an interrupt, so CPU has already
    ; switched to kernel mode and saved some state

    ; Save user context
    push ebp
    mov ebp, esp

    ; Assume the kernel has set up a pointer to where to save context
    mov edi, [current_context]

    ; Save general purpose registers
    mov [edi + CONTEXT.eax], eax
    mov [edi + CONTEXT.ebx], ebx
    mov [edi + CONTEXT.ecx], ecx
    mov [edi + CONTEXT.edx], edx
    mov [edi + CONTEXT.esi], esi
    mov eax, [ebp]
    mov [edi + CONTEXT.edi], eax  ; Original EDI
    mov eax, [ebp + 4]
    mov [edi + CONTEXT.ebp], eax  ; Original EBP
    mov eax, [ebp + 12]
    mov [edi + CONTEXT.esp], eax  ; Original ESP
    mov eax, [ebp + 8]
    mov [edi + CONTEXT.eip], eax  ; EIP
    pushfd
    pop eax
    mov [edi + CONTEXT.eflags], eax

    ; Save CR3
    mov eax, cr3
    mov [edi + CONTEXT.cr3], eax

    ; Load kernel segments
    mov ax, KERNEL_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Call C function to handle the kernel task
    extern kernel_task_handler
    call kernel_task_handler

    ; The kernel_task_handler should set up the next process to run
    ; and update current_context accordingly

    ; Restore context and return to user mode will be done by
    ; calling asm_switch_to_user from C code

    ; For now, just return to the interrupt handler
    pop ebp
    ret
