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
.section .data
.align 8
gdt:
    # Null descriptor (required first entry)
    .word 0x0000              # Limit 0-15
    .word 0x0000              # Base 0-15
    .byte 0x00                # Base 16-23
    .byte 0x00                # Access (all zero for null)
    .byte 0x00                # Granularity/Limit 16-19
    .byte 0x00                # Base 24-31
    
    # Code descriptor (ring 0, 4GB limit, 32-bit)
    # Access byte: [P=1][DPL=00][S=1][Type=1010] = 0x9A (executable, readable)
    # Flags byte: [G=1][DB=1][L=0][AVL=0][Limit 19-16=1111] = 0xCF (4KB pages, 32-bit)
    .word 0xFFFF              # Limit 0-15 (0xFFFF = 64KB, x4KB = 256MB per entry)
    .word 0x0000              # Base 0-15 (0x00000000)
    .byte 0x00                # Base 16-23
    .byte 0x9A                # Access: P=1, DPL=0, S=1, Type=1010 (code, readable)
    .byte 0xCF                # Flags: G=1, DB=1, L=0, AVL=0, Limit 19-16=1111
    .byte 0x00                # Base 24-31
    
    # Data descriptor (ring 0, 4GB limit, 32-bit)
    # Access byte: [P=1][DPL=00][S=1][Type=0010] = 0x92 (data, writable)
    # Flags byte: [G=1][DB=1][L=0][AVL=0][Limit 19-16=1111] = 0xCF (4KB pages, 32-bit)
    .word 0xFFFF              # Limit 0-15 (0xFFFF = 64KB, x4KB = 256MB per entry)
    .word 0x0000              # Base 0-15 (0x00000000)
    .byte 0x00                # Base 16-23
    .byte 0x92                # Access: P=1, DPL=0, S=1, Type=0010 (data, writable)
    .byte 0xCF                # Flags: G=1, DB=1, L=0, AVL=0, Limit 19-16=1111
    .byte 0x00                # Base 24-31
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt - 1   # GDT limit (size - 1)
    .long gdt                 # GDT linear base address

.set CODE_SEL, 0x08           # Index 1 (1 * 8 = 0x08)
.set DATA_SEL, 0x10           # Index 2 (2 * 8 = 0x10)

# --------------------------------------------------
# Stack
# --------------------------------------------------
.section .bss
.align 16
stack_bottom:
    .skip 65536                  # 64 KB stack (increased from 16KB for deeper call stacks)
stack_top: