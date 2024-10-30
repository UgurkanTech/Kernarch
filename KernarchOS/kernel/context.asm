section .text
global save_context
global load_context

; save_context(Context* context)
save_context:
    ; Push general-purpose registers and flags
    pushad                   ; Save all general-purpose registers (EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX)
    pushfd                   ; Save EFLAGS

    ; Get the context pointer from the stack
    mov eax, [esp + 44]      ; Context pointer is at [esp + 44]

    ; Save ESP (adjust for pushad and pushfd)
    lea edx, [esp + 8]       ; ESP + 8 to skip over saved pushad and pushfd values
    mov [eax + 12], edx      ; Save adjusted ESP in context

    ; Save general-purpose registers to context
    mov [eax + 0], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    mov [eax + 28], eax      ; EAX itself

    ; Save segment selectors
    mov edx, cs
    mov [eax + 32], dx
    mov edx, ds
    mov [eax + 34], dx
    mov edx, es
    mov [eax + 36], dx
    mov edx, fs
    mov [eax + 38], dx
    mov edx, gs
    mov [eax + 40], dx
    mov edx, ss
    mov [eax + 42], dx

    ; Save EIP (next instruction pointer) and EFLAGS
    mov edx, [esp + 4]       ; Return address is just above saved registers
    mov [eax + 44], edx      ; Save EIP in context
    mov edx, [esp]           ; EFLAGS was saved by pushfd
    mov [eax + 48], edx      ; Save EFLAGS

    ; Save control registers
    mov edx, cr0
    mov [eax + 52], edx
    mov edx, cr2
    mov [eax + 56], edx
    mov edx, cr3
    mov [eax + 60], edx
    mov edx, cr4
    mov [eax + 64], edx

    ; Restore original register state
    popfd                    ; Restore EFLAGS
    popad                    ; Restore all general-purpose registers
    ret


; load_context(Context* context)
; Does not return to caller, switches to new context
load_context:
    mov eax, [esp + 4]       ; Get context pointer from argument

    ; Load CR3 first if paging needs to change
    mov edx, [eax + 60]      ; offset 60 for CR3
    mov cr3, edx

    ; Load segment selectors from context
    mov dx, [eax + 34]       ; DS
    mov ds, dx
    mov dx, [eax + 36]       ; ES
    mov es, dx
    mov dx, [eax + 38]       ; FS
    mov fs, dx
    mov dx, [eax + 40]       ; GS
    mov gs, dx
    mov dx, [eax + 42]       ; SS
    mov ss, dx

    ; Set up the stack for iret
    mov esp, [eax + 12]    ; Load new stack pointer
    

    ; Load EFLAGS
    push dword [eax + 48]    ; offset 48 for EFLAGS
    popfd

    ; Prepare stack for iret (sets SS, ESP, EFLAGS, CS, EIP)
    push dword [eax + 42]    ; SS
    push dword [eax + 12]    ; ESP
    push dword [eax + 48]    ; EFLAGS
    push dword [eax + 32]    ; CS
    push dword [eax + 44]    ; EIP


    ; Load general-purpose registers
    mov edi, [eax + 0]     ; EDI
    mov esi, [eax + 4]     ; ESI
    mov ebp, [eax + 8]     ; EBP
    mov ebx, [eax + 16]    ; EBX
    mov edx, [eax + 20]    ; EDX
    mov ecx, [eax + 24]    ; ECX
    mov eax, [eax + 28]      ; EAX

    ; Execute iret to switch context
    iret
