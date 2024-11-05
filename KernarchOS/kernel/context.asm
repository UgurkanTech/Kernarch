section .text
global save_context
global load_context

; save_context(Context* context)
save_context:

    cli
    
    ; Load the context pointer from the stack
    mov eax, [esp + 4]             ; Get `Context* context` pointer from [esp + 4]

    ; Save all general-purpose registers
    pushad                        ; Push EDI, ESI, EBP, ESP (dummy), EBX, EDX, ECX, EAX in order

    ; Test value for EDI and ESI
    mov [eax + 0], edi   ; EDI
    mov [eax + 4], esi   ; ESI
    mov [eax + 8], ebp           ; EBP at Offset 8
    mov [eax + 12], esp          ; ESP at Offset 12 (current stack position)
    mov [eax + 16], ebx          ; EBX at Offset 16
    mov [eax + 20], edx          ; EDX at Offset 20
    mov [eax + 24], ecx          ; ECX at Offset 24
    mov [eax + 28], eax          ; EAX at Offset 28


    popad                         ; Restore all general-purpose registers

    ret


; load_context(Context* context)
; Does not return to caller, switches to new context
load_context:
    mov eax, [esp + 4]            ; Get context pointer from argument

    ; Load general-purpose registers
    mov edi, [eax + 0]            ; EDI at Offset 0
    mov esi, [eax + 4]            ; ESI at Offset 4
    mov ebp, [eax + 8]            ; EBP at Offset 8
    mov ebx, [eax + 16]           ; EBX at Offset 16
    mov edx, [eax + 20]           ; EDX at Offset 20
    mov ecx, [eax + 24]           ; ECX at Offset 24

    ; Load segment registers
    mov dx, [eax + 34]            ; DS at Offset 34
    mov ds, dx
    mov dx, [eax + 36]            ; ES at Offset 36
    mov es, dx
    mov dx, [eax + 38]            ; FS at Offset 38
    mov fs, dx
    mov dx, [eax + 40]            ; GS at Offset 40
    mov gs, dx

    ; Push stack, flags, and return info for both modes
    push dword [eax + 42]         ; SS
    push dword [eax + 12]         ; ESP
    push dword [eax + 48]         ; EFLAGS
    push dword [eax + 32]         ; CS
    push dword [eax + 44]         ; EIP
    mov eax, [eax + 28]           ; EAX at Offset 28       ; EAX at Offset 28

    sti                            ; Enable interrupts
    iretd                          ; Execute iret to switch context
