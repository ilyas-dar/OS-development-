#include <stdint.h>

/*

*  MODIFIER KEYS STATE

 * We remember these so letters
 * behave like a real keyboard.
 */

// shift pressed? (left or right)
static int shift = 0;

// caps lock toggled?
static int caps  = 0;

/*

*  PORT I/O
 * ============================
 * Talking to the keyboard
 * controller directly.
 */

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/*
 *  KERNEL FUNCTIONS
 * These live in kernel.c
 */

extern void putchar(char c);
extern void backspace(void);
extern void handle_command(void);

// shell command buffer
extern char cmd_buf[];
extern uint8_t cmd_len;

/*
 *  KEY MAPS
 * US QWERTY layout.
 * Scancode → character.
 */

static const char keymap[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,'*',0,' '
};

// same keys, but with shift pressed
static const char keymap_shift[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',
    0,'*',0,' '
};

/*
 * ============================
 *  KEYBOARD INTERRUPT HANDLER
 * Runs every time a key is
 * pressed or released.
 */

void keyboard_handler(void) {
    // read raw scancode from keyboard
    uint8_t sc = inb(0x60);

    /*
     * --- modifier keys ---
     */

    // shift pressed (left or right)
    if (sc == 0x2A || sc == 0x36) {
        shift = 1;
        goto done;
    }

    // shift released
    if (sc == 0xAA || sc == 0xB6) {
        shift = 0;
        goto done;
    }

    // caps lock toggles state
    if (sc == 0x3A) {
        caps = !caps;
        goto done;
    }

    /*
     * Ignore key release events.
     * We only care about key presses.
     */
    if (sc & 0x80)
        goto done;

    char c;

    /*
     * Letters need special handling:
     * shift XOR caps decides case.
     */
    if (sc >= 0x10 && sc <= 0x32)
        c = (shift ^ caps) ? keymap_shift[sc] : keymap[sc];
    else
        c = shift ? keymap_shift[sc] : keymap[sc];

    // unsupported key? just ignore it
    if (!c)
        goto done;

    /*
     * 
     *  CHARACTER HANDLING
     */

    if (c == '\b') {
        // backspace: shrink command buffer
        if (cmd_len > 0)
            cmd_len--;

        backspace();
    }
    else if (c == '\n') {
        // Enter key: finish command
        putchar('\n');
        handle_command();
    }
    else {
        // normal character
        if (cmd_len < 63)
            cmd_buf[cmd_len++] = c;

        putchar(c);
    }

done:
    /*
     * Tell the PIC we’re done
     * so it can send more interrupts.
     */
    outb(0x20, 0x20);
}