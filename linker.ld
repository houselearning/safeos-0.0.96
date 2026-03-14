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

  /* Align to page boundary for separate segment */
  . = ALIGN(4096);

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