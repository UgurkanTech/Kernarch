section .text
global load_context

; load_context(interrupt_frame* context)
; Does not return - switches directly to new context
load_context:
    cli                           ; Ensure interrupts are disabled
    mov eax, [esp + 4]           ; Get context pointer parameter
    
    ; Load all segment registers first
    mov dx, [eax + 16]           ; DS offset 16
    mov ds, dx
    mov dx, [eax + 12]           ; ES offset 12
    mov es, dx
    mov dx, [eax + 8]            ; FS offset 8
    mov fs, dx
    mov dx, [eax + 4]            ; GS offset 4
    mov gs, dx
    
    ; Load general registers
    mov edi, [eax + 20]          ; EDI offset 20
    mov esi, [eax + 24]          ; ESI offset 24
    mov ebx, [eax + 36]          ; EBX offset 36
    mov edx, [eax + 40]          ; EDX offset 40
    mov ecx, [eax + 44]          ; ECX offset 44
    
    ; Set up stack for iretd
    push dword [eax + 72]        ; SS offset 72
    push dword [eax + 68]        ; ESP offset 68
    push dword [eax + 64]        ; EFLAGS offset 64
    push dword [eax + 60]        ; CS offset 60
    push dword [eax + 56]        ; EIP offset 56
    
    ; Load EBP and EAX last
    mov ebp, [eax + 28]          ; EBP offset 28
    mov eax, [eax + 48]          ; EAX offset 48
    
    ; Switch context
    iretd