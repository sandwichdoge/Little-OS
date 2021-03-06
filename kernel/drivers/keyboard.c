#include "drivers/keyboard.h"

#include "builddef.h"
#include "drivers/io.h"
#include "syscall.h"  // _kb_handler_cb()

#define KBD_DATA_PORT 0x60

public
uint8_t read_scan_code() {
    return inb(KBD_DATA_PORT);
}

uint8_t sc_table_noshift[256] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    KEY_BACKSPACE /*Backspace*/,
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    17 /*LCtrl*/,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    KEY_LSHIFT /*LShift*/,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    KEY_RSHIFT /*RShift*/,
    0,
    0 /*LAlt*/,
    ' ',
    26 /*CapsLock*/,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 /*F10*/,
    0 /*NumLock*/,
    0 /*ScrollLock*/,
    0 /*Home*/,
    0 /*Up*/,
    0 /*PgUp*/,
    0 /*Keypad--*/,
    0 /*Left*/,
    0 /*Keypad-5*/,
    0 /*Right*/,
    0 /*Keypad-+*/,
    0 /*End*/,
    0 /*Down*/,
    0 /*PgDown*/,
    0 /*Insert*/,
    0 /*Keypad-Del*/,
    0,
    0,
    0,
    0 /*F11*/,
    0 /*F12*/
};

uint8_t sc_table_shift[256] = {
    0,
    27,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    KEY_BACKSPACE /*Backspace*/,
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n',
    17 /*LCtrl*/,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    KEY_LSHIFT /*LShift*/,
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    ',',
    '.',
    '/',
    KEY_RSHIFT /*RShift*/,
    0,
    0 /*LAlt*/,
    ' ',
    26 /*CapsLock*/,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 /*F10*/,
    0 /*NumLock*/,
    0 /*ScrollLock*/,
    0 /*Home*/,
    0 /*Up*/,
    0 /*PgUp*/,
    0 /*Keypad--*/,
    0 /*Left*/,
    0 /*Keypad-5*/,
    0 /*Right*/,
    0 /*Keypad-+*/,
    0 /*End*/,
    0 /*Down*/,
    0 /*PgDown*/,
    0 /*Insert*/,
    0 /*Keypad-Del*/,
    0,
    0,
    0,
    0 /*F11*/,
    0 /*F12*/
};

// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
public
uint8_t scan_code_to_ascii(uint8_t scan_code, int is_shift_depressed) {
    if (scan_code & (1 << 7)) {  // Key release
        // Ignore key release, unless it's SHIFT (sc_table_noshift[42]) then return another SHIFT
        if (scan_code == 170 || scan_code == 182) {
            return sc_table_noshift[scan_code ^ (1 << 7)];
        } else {
            return 0;
        }
    }

    if (is_shift_depressed) {
        return sc_table_shift[scan_code];
    } else {
        return sc_table_noshift[scan_code];
    }
}

private
void ISR_KEYBOARD(size_t* return_reg, struct cpu_state* unused) {
    static int is_shift_key_depressed = 0;
    unsigned char scan_code = read_scan_code();
    unsigned char ascii = scan_code_to_ascii(scan_code, is_shift_key_depressed);
    if (ascii == KEY_LSHIFT || ascii == KEY_RSHIFT) {
        is_shift_key_depressed ^= 1;
    } else if (_kb_handler_cb) {
        _kb_handler_cb(ascii);
    }
}

public
void keyboard_init() {
    interrupt_register(INT_KEYBOARD, ISR_KEYBOARD);
}
