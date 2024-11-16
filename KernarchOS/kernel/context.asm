section .text
global load_context

; load_context(Context* context)
; Does not return to caller, switches to new context
load_context:
    cli
    mov eax, [esp + 4]            ; Get context pointer from argument

    ; Load general-purpose registers
    mov edi, [eax + 20]           ; EDI at Offset 20
    mov esi, [eax + 24]           ; ESI at Offset 24
    mov ebp, [eax + 28]           ; EBP at Offset 28
    mov ebx, [eax + 36]           ; EBX at Offset 36
    mov edx, [eax + 40]           ; EDX at Offset 40
    mov ecx, [eax + 44]           ; ECX at Offset 44

    ; Load segment registers
    mov dx, [eax + 16]            ; DS at Offset 16
    mov ds, dx
    mov dx, [eax + 12]            ; ES at Offset 12
    mov es, dx
    mov dx, [eax + 8]             ; FS at Offset 8
    mov fs, dx
    mov dx, [eax + 4]             ; GS at Offset 4
    mov gs, dx

    ; Push stack, flags, and return info for both modes
    push dword [eax + 72]         ; SS at Offset 72
    push dword [eax + 68]         ; ESP at Offset 68
    push dword [eax + 64]         ; EFLAGS at Offset 64
    push dword [eax + 60]         ; CS at Offset 60
    push dword [eax + 56]         ; EIP at Offset 56
    mov eax, [eax + 48]           ; EAX at Offset 48
    
    iretd                          ; Execute iret to switch context