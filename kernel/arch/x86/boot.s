.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_FLAGS, 0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

/* Place multiboot header in its own allocatable section so the
    linker will emit it at the start of the output file (within the
    first 8 KiB). Marking the section as allocatable ensures it is
    included in the ELF program headers and visible to GRUB. */
.section .multiboot,"aw",@progbits
     .align 4
     .long MULTIBOOT_MAGIC
     .long MULTIBOOT_FLAGS
     .long MULTIBOOT_CHECKSUM

.section .text
    .global _start
_start:
    cli
    mov $stack_top, %esp
    /* Proceed to kernel entry (multiboot registers expected from bootloader) */
    push %ebx          # multiboot info
    push %eax          # multiboot magic
    call kmain
.hang:
    cli
    hlt
    jmp .hang

.section .bss
    .align 16
stack_bottom:
    .skip 16384
stack_top:
