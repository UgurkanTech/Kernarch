; context_switch.asm
section .text
global save_context
global load_context

save_context:
    ; Get the Context* argument
    mov eax, [esp + 4]
    
    ; Save general purpose registers
    mov [eax + 0], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov [eax + 12], esp
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    mov [eax + 28], eax
    
    ; Save segment registers
    mov [eax + 32], cs
    mov [eax + 34], ds
    mov [eax + 36], es
    mov [eax + 38], fs
    mov [eax + 40], gs
    mov [eax + 42], ss
    
    ; Save eip (return address)
    mov ecx, [esp]
    mov [eax + 44], ecx
    
    ; Save eflags
    pushfd
    pop ecx
    mov [eax + 48], ecx
    
    ; Save CR3 (page directory)
    mov ecx, cr3
    mov [eax + 52], ecx
    
    ret

load_context:
    ; Get the Context* argument
    mov eax, [esp + 4]
    
    ; Load CR3 (page directory)
    mov ecx, [eax + 52]
    mov cr3, ecx
    
    ; Load segment registers
    mov cx, [eax + 34]
    mov ds, cx
    mov cx, [eax + 36]
    mov es, cx
    mov cx, [eax + 38]
    mov fs, cx
    mov cx, [eax + 40]
    mov gs, cx
    
    ; Load general purpose registers
    mov edi, [eax + 0]
    mov esi, [eax + 4]
    mov ebp, [eax + 8]
    mov ebx, [eax + 16]
    mov edx, [eax + 20]
    mov ecx, [eax + 24]
    
    ; Load esp
    mov esp, [eax + 12]
    
    ; Push ss (if switching to user mode)
    push dword [eax + 42]
    
    ; Push esp
    push dword [eax + 12]
    
    ; Push eflags
    push dword [eax + 48]
    
    ; Push cs
    push dword [eax + 32]
    
    ; Push eip
    push dword [eax + 44]
    
    ; Load eax last (since we've been using it)
    mov eax, [eax + 28]
    
    ; Return and switch context
    iretd