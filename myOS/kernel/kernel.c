

#include <stdint.h>
#include <stdarg.h>

/* ---------- VGA ---------- */

static uint16_t* const VGA = (uint16_t*)0xB8000;
static uint8_t row = 0;
static uint8_t col = 0;
static const uint8_t COLOR = 0x0F;

static void putchar(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        return;
    }

    VGA[row * 80 + col] = ((uint16_t)COLOR << 8) | c;
    col++;

    if (col >= 80) {
        col = 0;
        row++;
    }
}

static void print(const char* s) {
    while (*s) putchar(*s++);
}

static void print_int(int x) {
    char buf[12];
    int i = 0;

    if (x == 0) {
        putchar('0');
        return;
    }

    if (x < 0) {
        putchar('-');
        x = -x;
    }

    while (x > 0) {
        buf[i++] = '0' + (x % 10);
        x /= 10;
    }

    while (i--) putchar(buf[i]);
}

/* ---------- retroput ---------- */

void retroput(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            putchar(*fmt);
            continue;
        }

        fmt++;
        if (*fmt == 's') {
            print(va_arg(args, char*));
        } else if (*fmt == 'c') {
            putchar((char)va_arg(args, int));
        } else if (*fmt == 'd') {
            print_int(va_arg(args, int));
        } else {
            putchar('%');
            putchar(*fmt);
        }
    }

    va_end(args);
}

/* ----- kernel --*/
const char* logo =
"                          ____      _              \n"
"                         |  _ \\ ___| |_ _ __ ___  \n"
"                         | |_) / _ \\ __| '__/ _ \\ \n"
"                         |  _ <  __/ |_| | | |_| | \n"
"                         |_| \\_\\___|\\__|_|  \\___/ \n";



void kernel_main(void) {
    retroput("%s", logo);
    

    while (1) {}
}
