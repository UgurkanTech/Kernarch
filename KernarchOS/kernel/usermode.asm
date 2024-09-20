global jump_to_user_mode

USER_CODE_SEG equ 0x1B  ; User code segment (index 3, RPL 3)
USER_DATA_SEG equ 0x23  ; User data segment (index 4, RPL 3)

jump_to_user_mode:
    mov eax, [esp + 4]  ; entry point
    mov ebx, [esp + 8]  ; user stack

    ; Set up the stack frame for iret
    push dword USER_DATA_SEG  ; user data segment
    push ebx            ; user stack
    pushf               ; push EFLAGS
    pop ecx
    or ecx, 0x200       ; set IF flag (enable interrupts)
    push ecx            ; push modified EFLAGS
    push dword USER_CODE_SEG  ; user code segment
    push eax            ; entry point

    ; Set segment registers
    mov ax, USER_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Jump to user mode
    iret