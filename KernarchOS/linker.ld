ENTRY(_start)

KERNEL_START = 0x100000;


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
}