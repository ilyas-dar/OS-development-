#include <stdint.h>
#include <stdarg.h>
#include "idt.h"

/* ---------- VGA ---------- */

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define LOGO_HEIGHT 7
#define SCROLL_START LOGO_HEIGHT
#define SCROLL_END   (VGA_HEIGHT - 1)

#define VGA_COLOR(fg, bg) ((uint8_t)(((bg) << 4) | (fg)))

static uint16_t* const VGA = (uint16_t*)0xB8000;

static uint8_t row = 0;
static uint8_t col = 0;
static uint8_t input_row = 0;
static uint8_t input_col = 0;
static uint8_t color = VGA_COLOR(0xF, 0x0);

static const char* PROMPT = "RetroOS> ";

/* ---------- command buffer ---------- */

#define CMD_BUF_SIZE 64
char cmd_buf[CMD_BUF_SIZE];
uint8_t cmd_len = 0;

/* ---------- I/O ---------- */

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ---------- cursor ---------- */

static void update_cursor(void) {
    uint16_t pos = row * VGA_WIDTH + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

/* ---------- color ---------- */

void set_color(uint8_t fg, uint8_t bg) {
    color = VGA_COLOR(fg, bg);
}

/* ---------- scroll ---------- */

static void scroll_if_needed(void) {
    if (row < SCROLL_END)
        return;

    for (int y = SCROLL_START; y < SCROLL_END; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA[y * VGA_WIDTH + x] =
                VGA[(y + 1) * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA[SCROLL_END * VGA_WIDTH + x] =
            ((uint16_t)color << 8) | ' ';
    }

    row = SCROLL_END;
}

/* ---------- prompt ---------- */

void print_prompt(void) {
    set_color(0xA, 0x0);

    for (const char* p = PROMPT; *p; p++) {
        VGA[row * VGA_WIDTH + col] =
            ((uint16_t)color << 8) | *p;
        col++;
    }

    set_color(0xF, 0x0);

    input_row = row;
    input_col = col;
    update_cursor();
}

/* ---------- backspace ---------- */

void backspace(void) {
    if (row < input_row ||
       (row == input_row && col <= input_col))
        return;

    if (col == 0) {
        row--;
        col = VGA_WIDTH - 1;
    } else {
        col--;
    }

    VGA[row * VGA_WIDTH + col] =
        ((uint16_t)color << 8) | ' ';

    update_cursor();
}

/* ---------- putchar ---------- */

void putchar(char c) {
    if (c == '\b') {
        backspace();
        return;
    }

    if (c == '\n') {
        col = 0;
        row++;
        scroll_if_needed();
        update_cursor();
        return;
    }

    VGA[row * VGA_WIDTH + col] =
        ((uint16_t)color << 8) | c;

    col++;

    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
        scroll_if_needed();
    }

    update_cursor();
}

/* ---------- printing ---------- */

static void print(const char* s) {
    while (*s) putchar(*s++);
}

void retroput(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            putchar(*fmt);
            continue;
        }
        fmt++;
        if (*fmt == 's') print(va_arg(args, char*));
        else if (*fmt == 'c') putchar(va_arg(args, int));
    }

    va_end(args);
}

/* ---------- helpers ---------- */

int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

void clear_screen(void) {
    for (int y = SCROLL_START; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA[y * VGA_WIDTH + x] =
                ((uint16_t)color << 8) | ' ';

    row = SCROLL_START;
    col = 0;
}

/* ---------- shell ---------- */

void handle_command(void) {
    cmd_buf[cmd_len] = 0;

    if (cmd_len == 0) {
        print_prompt();
        return;
    }

    if (streq(cmd_buf, "help")) {
        retroput("Available commands:\n");
        retroput("  help  - show this message\n");
        retroput("  clear - clear the screen\n");
    } else if (streq(cmd_buf, "clear")) {
        clear_screen();
    } else {
        retroput("Unknown command: %s\n", cmd_buf);
    }

    cmd_len = 0;
    print_prompt();
}

/* ---------- logo ---------- */

const char* logo =
"                    >=>                          >===>        >=>>=>   \n"
"                    >=>                        >=>    >=>   >=>    >=> \n"
">> >==>   >==>    >=>>==> >> >==>    >=>     >=>        >=>  >=>       \n"
" >=>    >>   >=>    >=>    >=>     >=>  >=>  >=>        >=>    >=>     \n"
" >=>    >>===>>=>   >=>    >=>    >=>    >=> >=>        >=>       >=>  \n"
" >=>    >>          >=>    >=>     >=>  >=>    >=>     >=>  >=>    >=> \n"
">==>     >====>      >=>  >==>       >=>         >===>        >=>>=>   \n";

/* ---------- kernel ---------- */

void kernel_main(void) {
    row = 0;
    col = 0;

    set_color(0xE, 0x0);
    retroput("%s", logo);

    /* CLEAR scroll region ONCE (IMPORTANT FIX) */
    for (int y = SCROLL_START; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA[y * VGA_WIDTH + x] =
                ((uint16_t)VGA_COLOR(0xF, 0x0) << 8) | ' ';

    row = SCROLL_START;
    col = 0;

    idt_init();
    __asm__ volatile ("sti");

    print_prompt();

    while (1)
        __asm__ volatile ("hlt");
}