/* Multiboot kernel linker script for binary output
   Ensures Multiboot header is at absolute file offset 0 */

OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
  /* Load at 64KB initially (within identity-mapped region before paging)
     Later code will set up paging if needed */
  . = 0x00010000;

  .text : ALIGN(16)
  {
    KEEP(*(.multiboot))
    *(.text*)
    *(.gnu.linkonce.t.*)
  }

  .rodata : ALIGN(16)
  {
    *(.rodata*)
    *(.gnu.linkonce.r.*)
  }

  .data : ALIGN(16)
  {
    *(.data*)
    *(.gnu.linkonce.d.*)
  }

  .bss : ALIGN(16)
  {
    _bss_start = .;
    *(.bss*)
    *(.gnu.linkonce.b.*)
    _bss_end = .;
  }

  _end = .;
}