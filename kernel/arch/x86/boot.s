.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_FLAGS, 0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

/* Emit the Multiboot header inside the first loadable section (.text)
   so it resides within the first 8 KiB of the file and is in a PT_LOAD
   program header for GRUB to find. */
.section .text
    .align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    .global _start
_start:
    cli
    mov $stack_top, %esp
    /* Very early serial tick from assembly so host sees kernel start */
    mov $0x3f8, %dx
    mov $'S', %al
    outb %al, %dx
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
