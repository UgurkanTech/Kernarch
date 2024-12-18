section .text
%macro int_vector_macro 1
  int_vector_handler_%1:
  cli
  %if !(%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 30)
    push 0xFFFFFFFF ; dummy error code
  %endif

  pushad

  push ds
  push es
  push fs
  push gs

  push esp
  cld
  push %1
  jmp interrupt_common
%endmacro

extern isr_handler

null_loop:
    hlt  ; Halt the CPU until the next interrupt
    jmp null_loop

; Special handler for interrupt 255 (simply halts the CPU) 
int_vector_handler_255:
  cli
  add esp, 12 ;Check this again
  sti
  jmp null_loop
  iret

interrupt_common:
  call isr_handler

  add esp, 4 ; pop %1
  pop esp

  pop gs
  pop fs
  pop es
  pop ds

  popad

  add esp, 4 ; pop error code
  iret

%assign i 0
%rep 255
  int_vector_macro i
  %assign i i + 1
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dd int_vector_handler_%+i
%assign i i+1
%endrep