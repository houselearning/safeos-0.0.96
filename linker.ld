ENTRY(_start)

SECTIONS
{
  .multiboot :
  {
    *(.multiboot)
  }

  . = 1M;

  .text :
  {
    *(.text*)
  }

  .rodata :
  {
    *(.rodata*)
  }

  .data :
  {
    *(.data*)
  }

  .bss :
  {
    *(.bss*)
    *(COMMON)
  }
}
