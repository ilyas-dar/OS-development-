#include <stdint.h>

/* modifier state */
static int shift = 0;
static int caps  = 0;

/* I/O */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* from kernel */
extern void putchar(char c);
extern void backspace(void);
extern void handle_command(void);
extern char cmd_buf[];
extern uint8_t cmd_len;

/* keymaps */
static const char keymap[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,'*',0,' '
};

static const char keymap_shift[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',
    0,'*',0,' '
};

void keyboard_handler(void) {
    uint8_t sc = inb(0x60);

    if (sc == 0x2A || sc == 0x36) { shift = 1; goto done; }
    if (sc == 0xAA || sc == 0xB6) { shift = 0; goto done; }
    if (sc == 0x3A) { caps = !caps; goto done; }

    if (sc & 0x80)
        goto done;

    char c;

    if (sc >= 0x10 && sc <= 0x32)
        c = (shift ^ caps) ? keymap_shift[sc] : keymap[sc];
    else
        c = shift ? keymap_shift[sc] : keymap[sc];

    if (!c)
        goto done;

    if (c == '\b') {
        if (cmd_len > 0) cmd_len--;
        backspace();
    }
    else if (c == '\n') {
        putchar('\n');
        handle_command();
    }
    else {
        if (cmd_len < 63)
            cmd_buf[cmd_len++] = c;
        putchar(c);
    }

done:
    outb(0x20, 0x20);
}