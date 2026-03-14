.set MULTIBOOT_MAGIC,    0x1BADB002
.set MULTIBOOT_FLAGS,    0x00000003  # request memory info + video mode
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# --------------------------------------------------
# Multiboot header (MUST be early & loadable)
# --------------------------------------------------
.section .multiboot
.align 4
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM
# (Additional fields are only needed if you set the corresponding flags.)
# .long 0  # header_addr
# .long 0  # load_addr
# .long 0  # load_end_addr
# .long 0  # bss_end_addr
# .long 0  # entry_addr
# .long 0  # mode_type: 0 = graphics
# .long 1024  # width
# .long 768   # height
# .long 32    # depth

# --------------------------------------------------
# Code
# --------------------------------------------------
.section .text
.global _start
.extern kmain

_start:
    cli                         # interrupts OFF

    # Load our own GDT
    lgdt gdt_descriptor

    # Reload data segments
    mov $DATA_SEL, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # Reload CS via far jump
    ljmp $CODE_SEL, $flush_cs

flush_cs:
    # Valid stack AFTER SS is loaded
    mov $stack_top, %esp

    # Early serial tick (COM1)
    mov $0x3F8, %dx
    mov $'S', %al
    outb %al, %dx

    # Multiboot ABI:
    # eax = magic
    # ebx = multiboot info pointer
    push %ebx
    push %eax
    call kmain

.hang:
    cli
    hlt
    jmp .hang

# --------------------------------------------------
# Global Descriptor Table
# --------------------------------------------------
.section .rodata
gdt:
    .quad 0x0000000000000000      # null descriptor
    .quad 0x00CF9A000000FFFF      # code segment (base=0, limit=4GB)
    .quad 0x00CF92000000FFFF      # data segment (base=0, limit=4GB)
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt - 1
    .long gdt

.set CODE_SEL, 0x08
.set DATA_SEL, 0x10

# --------------------------------------------------
# Stack
# --------------------------------------------------
.section .bss
.align 16
stack_bottom:
    .skip 16384                  # 16 KB stack
stack_top: