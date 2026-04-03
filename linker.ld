ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

SECTIONS
{
  /* The multiboot header MUST appear at the very beginning of the output file.
     We don't set a fixed address here - let it be at offset 0 in the file. */
  
  .multiboot : 
  {
    KEEP(*(.multiboot))
  }

  /* All other sections start at 1MB in virtual address space */
  . = 1M;

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

  /* Uninitialized data and stack */
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

  /* Discard unnecessary sections */
  /DISCARD/ :
  {
    *(.note.GNU-stack)
    *(.gnu_hash)
    *(.gnu.version*)
    *(.interp)
    *(.comment)
  }
}