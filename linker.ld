ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

SECTIONS
{
  /* The multiboot header MUST be at file offset 0-8KB.
     Use AT(0) to place multiboot at the absolute beginning of the file,
     while keeping other sections at 1MB virtual address. */
  
  .multiboot 0x100000 : AT(0)
  {
    KEEP(*(.multiboot))
  }

  /* Ensure other sections also start at 1MB in virtual address space */
  . = 0x100000 + SIZEOF(.multiboot);

  /* Code section */
  .text :
  {
    *(.text*)
    *(.gnu.linkonce.t.*)
  }

  /* Read-only data */
  .rodata :
  {
    *(.rodata*)
    *(.gnu.linkonce.r.*)
  }

  /* Initialized data */
  .data :
  {
    *(.data*)
    *(.gnu.linkonce.d.*)
  }

  _edata = .;

  /* Uninitialized data */
  .bss :
  {
    _bss_start = .;
    *(.bss*)
    *(COMMON)
    *(.gnu.linkonce.b.*)
    _bss_end = .;
  }

  _end = .;

  /* Debug sections (not loaded) */
  .debug_info 0 : { *(.debug_info) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_str 0 : { *(.debug_str) }

  /* Discard */
  /DISCARD/ :
  {
    *(.note.GNU-stack)
    *(.gnu_hash)
    *(.gnu.version*)
    *(.interp)
    *(.comment)
  }
}