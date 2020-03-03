#include "syscall.h" // _kb_handler_cb()
#include "interrupt.h"
#include "pic.h"
#include "keyboard.h"
#include "utils/debug.h"

extern void asm_int_handler_33(); // Handler for keyboard press
extern void asm_int_handler_14(); // Handler for page fault

#define IDT_SIZE 256
#define INT_PAGEFAULT 14
#define INT_KEYBOARD 33 // 0x20 + 1

struct cpu_state {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
};

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
};

struct idt IDT; // To be loaded into the CPU
struct idt_entry idt_entries[IDT_SIZE] = {0}; // Main content of IDT
void (*int_handler_table[IDT_SIZE])(void) = {0}; // Array of void func(void) pointers

// https://wiki.osdev.org/Interrupt_Descriptor_Table
void interrupt_encode_idt_entry(unsigned int interrupt_num, unsigned int f_ptr_handler) {
    idt_entries[interrupt_num].offset_low = f_ptr_handler & 0xffff;
    idt_entries[interrupt_num].offset_high = (f_ptr_handler >> 16) & 0xffff;

    idt_entries[interrupt_num].segment_selector = 0x8; // code segment in gdt
    idt_entries[interrupt_num].reserved = 0x0;

    idt_entries[interrupt_num].type_and_attr =  (1 << 7) |  // P
                                                (0 << 6) | (0 << 5) |	// DPL
                                                (0 << 4) |  // S
                                                0xe;        // 32-bit interrupt gate - 0b1110
}

void lidt (struct idt *idt_r)
{
    asm ("lidt %0" :: "m"(*idt_r));
}

void ISR_KEYBOARD(void) {
    static int is_shift_key_depressed = 0;
    unsigned char scan_code = read_scan_code();
    unsigned char ascii = scan_code_to_ascii(scan_code, is_shift_key_depressed);
    if (ascii == KEY_LSHIFT || ascii == KEY_RSHIFT) {
        is_shift_key_depressed ^= 1;
    } else {
        _kb_handler_cb(ascii);
    }
}

void ISR_PAGEFAULT(void) {
    _dbg_set_edi_esi(0x555);
    _dbg_break();
}

void interrupt_init_idt(void) {
    /*
    IRQ 0 ‒ system timer
    IRQ 1 — keyboard controller
    IRQ 3 — serial port COM2
    IRQ 4 — serial port COM1
    IRQ 5 — line print terminal 2
    IRQ 6 — floppy controller
    IRQ 7 — line print terminal 1
    IRQ 8 — RTC timer
    IRQ 12 — mouse controller
    IRQ 13 — math co-processor
    IRQ 14 — ATA channel 1
    IRQ 15 — ATA channel 2
    */

    // Init ISR table
    int_handler_table[INT_KEYBOARD] = ISR_KEYBOARD;
    int_handler_table[INT_PAGEFAULT] = ISR_PAGEFAULT;

    // Keyboard press interrupt, 0x20 + 1 (which is PIC1_START_INTERRUPT + IRQ_1)
    interrupt_encode_idt_entry(INT_KEYBOARD, (unsigned int)asm_int_handler_33);
    interrupt_encode_idt_entry(INT_PAGEFAULT, (unsigned int)asm_int_handler_14);

    IDT.size = sizeof(struct idt_entry) * IDT_SIZE - 1;
    IDT.address = (unsigned int)idt_entries;

    lidt(&IDT); // ASM wrapper
}

void interrupt_handler(struct cpu_state cpu_state, unsigned int interrupt_num, struct stack_state stack_state) {
    if (interrupt_num >= sizeof(int_handler_table) / sizeof(*int_handler_table)) {
        return; // Stop if array out of range
    }

    if (int_handler_table[interrupt_num]) {
        (*int_handler_table[interrupt_num])();
        pic_ack(interrupt_num);
    }
}