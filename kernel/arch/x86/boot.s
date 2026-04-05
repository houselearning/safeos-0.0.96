.set MULTIBOOT_MAGIC,    0x1BADB002
.set MULTIBOOT_FLAGS,    0x00000003  # request module align + memory info (NO video mode - avoid GRUB graphics issues)
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# --------------------------------------------------
# Multiboot header (MUST be early & loadable)
# --------------------------------------------------
.section .multiboot
.align 4
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM
.long 0  # header_addr (unused)
.long 0  # load_addr (leave loader defaults)
.long 0  # load_end_addr
.long 0  # bss_end_addr
.long 0  # entry_addr

# --------------------------------------------------
# Code
# --------------------------------------------------
.section .text
.global _start
.extern kmain

_start:
    # ===========================================
    # CHECKPOINT 0: Entry point
    # Absolutely minimal code - no BIOS calls
    # ===========================================
    
    # Write directly to VGA memory (0xB8000)
    # This is the most reliable method
    mov $0xB8000, %ecx
    
    # "E" at offset 0
    mov $0x0F45, %ax
    mov %ax, (%ecx)
    
    # "N" at offset 2  
    mov $0x0F4E, %ax
    mov %ax, 2(%ecx)
    
    # "T" at offset 4
    mov $0x0F54, %ax
    mov %ax, 4(%ecx)
    
    # "R" at offset 6
    mov $0x0F52, %ax
    mov %ax, 6(%ecx)
    
    # Serial output - COM1 port 0x3F8
    # This works in any mode
    mov $0x3F8, %dx
    mov $'E', %al
    outb %al, %dx
    mov $'N', %al
    outb %al, %dx
    mov $'T', %al
    outb %al, %dx
    mov $'R', %al
    outb %al, %dx
    
    # Now disable interrupts
    cli

    # Load our own GDT
    # During boot, we're loaded at 0x10000, so labels should have correct addresses
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
    # ===========================================
    # CHECKPOINT 1: After segment reload
    # ===========================================
    mov $0xB8000, %ecx
    mov $0x0F31, %ax           # '1' in white on black
    mov %ax, 8(%ecx)
    
    # Serial: send '1'
    mov $0x3F8, %dx
    mov $'1', %al
    outb %al, %dx
    # Valid stack AFTER SS is loaded
    mov $stack_top, %esp

    # ===========================================
    # CHECKPOINT 2: Stack setup complete
    # ===========================================
    mov $0xB8000, %ecx
    mov $0x0F32, %ax           # '2' in white
    mov %ax, 10(%ecx)

    # Multiboot ABI:
    # eax = magic
    # ebx = multiboot info pointer
    push %ebx
    push %eax
    call kmain
    
    # Should never return from kmain
    cli
    hlt
    jmp .

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