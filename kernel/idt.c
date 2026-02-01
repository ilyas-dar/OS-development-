#include "idt.h"

/* I/O */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* IDT entry */
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];

extern void keyboard_isr(void);  // from ASM

/* PIC remap */
static void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFD); // enable IRQ1
    outb(0xA1, 0xFF);
}

static void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;   // kernel code segment
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;   // interrupt gate
    idt[n].offset_high = handler >> 16;
}

static void idt_load(void) {
    struct idt_ptr idtp;
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

void idt_init(void) {
    pic_remap();
    idt_set_gate(0x21, (uint32_t)keyboard_isr);
    idt_load();
}