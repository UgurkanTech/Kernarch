ENTRY(_start)

KERNEL_START = 0x100000;
KERNEL_STACK_SIZE = 65536;  /* 64 KB */
STACK_GUARD_SIZE = 1M;      /* 1 MB guard region */
KERNEL_HEAP_SIZE = 12M;      /* 12 MB heap */

SECTIONS
{
  . = KERNEL_START;
  kernel_start = .;

  .text : AT(ADDR(.text))
  {
    *(.multiboot)
    *(.text)
  }
  .rodata : AT(ADDR(.rodata))
  {
    *(.rodata*)
  }
  .data : AT(ADDR(.data))
  {
    *(.data)
  }
  .bss : AT(ADDR(.bss))
  {
    *(.COMMON)
    *(.bss)
  }
  kernel_end = .;


  /* Stack guard region */
  . = ALIGN(4K);
  stack_guard_bottom = .;
  .stack_guard : AT(ADDR(.stack_guard))
  {
    . += STACK_GUARD_SIZE;
  }
  stack_guard_top = .;

  /* Stack section */
  . = ALIGN(4K);
  stack_bottom = .;
  .stack : AT(ADDR(.stack))
  {
    . += KERNEL_STACK_SIZE;
  }
  stack_top = .;

  /* Heap section */
  . = ALIGN(4K);
  heap_start = .;
  .heap : AT(ADDR(.heap))
  {
    . += KERNEL_HEAP_SIZE;
  }
  heap_end = .;
}