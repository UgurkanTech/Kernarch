section .text
global load_context

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
