; Context switching implementation for x86
section .text
global save_context
global load_context

; save_context(Context* context)
; Returns: 0 when saving context, 1 when returning from load_context
save_context:
    ; Check if context pointer is null
    mov eax, [esp + 4]
    test eax, eax
    jz .skip_save        ; If null, skip saving
    
    ; Save general purpose registers
    mov [eax + 0], edi   ; offset 0
    mov [eax + 4], esi   ; offset 4
    mov [eax + 8], ebp   ; offset 8
    mov [eax + 16], ebx  ; offset 16
    mov [eax + 20], edx  ; offset 20
    mov [eax + 24], ecx  ; offset 24
    
    ; Save esp (adjust for the call to save_context)
    lea edx, [esp + 8]   ; esp + 8 to skip return addr and context ptr
    mov [eax + 12], edx  ; offset 12
    
    ; Save eax
    mov [eax + 28], eax  ; offset 28
    
    ; Save segment registers
    mov edx, cs
    mov [eax + 32], edx  ; offset 32
    mov edx, ds
    mov [eax + 34], edx  ; offset 34
    mov edx, es
    mov [eax + 36], edx  ; offset 36
    mov edx, fs
    mov [eax + 38], edx  ; offset 38
    mov edx, gs
    mov [eax + 40], edx  ; offset 40
    mov edx, ss
    mov [eax + 42], edx  ; offset 42
    
    ; Save eip (return address)
    mov edx, [esp]       ; Get return address from stack
    mov [eax + 44], edx  ; offset 44
    
    ; Save eflags
    pushfd
    pop edx
    mov [eax + 48], edx  ; offset 48
    
    ; Save control registers
    mov edx, cr3
    mov [eax + 60], edx  ; offset 60
    
.skip_save:
    xor eax, eax        ; Return 0 when saving context
    ret

; load_context(Context* context)
; Never returns to caller - switches to new context
load_context:
    mov eax, [esp + 4]   ; Get context pointer
    
    ; Load CR3 first if paging needs to change
    mov edx, [eax + 60]  ; offset 60
    mov cr3, edx
    
    ; Load segment registers
    mov dx, [eax + 34]   ; offset 34
    mov ds, dx
    mov dx, [eax + 36]   ; offset 36
    mov es, dx
    mov dx, [eax + 38]   ; offset 38
    mov fs, dx
    mov dx, [eax + 40]   ; offset 40
    mov gs, dx
    mov dx, [eax + 42]   ; offset 42
    mov ss, dx
    
    ; Load general registers
    mov ebp, [eax + 8]   ; offset 8
    mov ebx, [eax + 16]  ; offset 16
    mov edx, [eax + 20]  ; offset 20
    mov ecx, [eax + 24]  ; offset 24
    mov esi, [eax + 4]   ; offset 4
    mov edi, [eax + 0]   ; offset 0
    
    ; Load eflags
    push dword [eax + 48] ; offset 48
    popfd
    
    ; Set up stack for iret
    push dword [eax + 42] ; SS
    push dword [eax + 12] ; ESP
    push dword [eax + 48] ; EFLAGS
    push dword [eax + 32] ; CS
    push dword [eax + 44] ; EIP
    
    ; Load eax last (no longer need context pointer)
    mov eax, [eax + 28]  ; offset 28
    
    ; Switch to new context
    iret