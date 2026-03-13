ENTRY(_start)

SECTIONS
{
  /* Kernel load address */
  . = 1M;

  /* Multiboot header MUST be early and loadable */
  .text :
  {
    KEEP(*(.multiboot))
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
    *(COMMON)
    *(.bss*)
  }
}