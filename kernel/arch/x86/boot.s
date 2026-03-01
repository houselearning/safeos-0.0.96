.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_FLAGS, 0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

.section .text
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    .global _start
_start:
    cli
    mov $stack_top, %esp
    /* Aggressive serial probe: repeatedly write '>' + CR to COM1 (0x3F8)
       and hang so we can verify the kernel entry is reached. */
    mov $0x3f8, %dx
.serial_loop:
    mov $'>', %al
    outb %al, %dx
    mov $0x0d, %al
    outb %al, %dx
    /* small busy delay */
    mov $0x200000, %ecx
.delay_loop:
    dec %ecx
    jne .delay_loop
    jmp .serial_loop

    /* Not reached while loop active, kept for reference */
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
