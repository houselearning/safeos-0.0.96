#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// IDT entry structure
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

// IDT pointer
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// Initialize interrupt handling
void interrupts_init(void);

// Set IDT entry
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Load IDT
void idt_load(void);

// PIC functions
void pic_remap(void);
void pic_send_eoi(uint8_t irq);

// Interrupt handlers
void isr0();  // Divide by zero
void isr1();  // Debug
void isr2();  // NMI
void isr3();  // Breakpoint
void isr4();  // Overflow
void isr5();  // Bound range
void isr6();  // Invalid opcode
void isr7();  // Device not available
void isr8();  // Double fault
void isr9();  // Coprocessor segment overrun
void isr10(); // Invalid TSS
void isr11(); // Segment not present
void isr12(); // Stack fault
void isr13(); // General protection
void isr14(); // Page fault
void isr15(); // Reserved
void isr16(); // Floating point
void isr17(); // Alignment check
void isr18(); // Machine check
void isr19(); // SIMD floating point
void isr20(); // Virtualization
void isr21(); // Reserved
void isr22(); // Reserved
void isr23(); // Reserved
void isr24(); // Reserved
void isr25(); // Reserved
void isr26(); // Reserved
void isr27(); // Reserved
void isr28(); // Reserved
void isr29(); // Reserved
void isr30(); // Reserved
void isr31(); // Reserved

// IRQ handlers
void irq0();  // Timer
void irq1();  // Keyboard
void irq2();  // Cascade
void irq3();  // COM2
void irq4();  // COM1
void irq5();  // LPT2
void irq6();  // Floppy
void irq7();  // LPT1
void irq8();  // RTC
void irq9();  // Reserved
void irq10(); // Reserved
void irq11(); // Reserved
void irq12(); // Mouse
void irq13(); // FPU
void irq14(); // Primary ATA
void irq15(); // Secondary ATA

#endif
