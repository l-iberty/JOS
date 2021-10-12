#include <include/lib.h>
#include <include/types.h>
#include <include/memlayout.h>

#include <kernel/console.h>

static void cga_init();
static void cga_putc();
static void cga_newline();
static void cga_print();

void cons_init() {
  cga_init();
}

void cons_print(const char *s) {
  cga_print(s);
}

static uint16_t *crt_buf;
static uint16_t crt_pos;

static void cga_init() {
  uint16_t pos;

  crt_buf = (uint16_t *)(CGA_BASE + KERNBASE);

  // extract cursor location
  outb(CRT_ADDR, CRT_CURSOR_HIGH);
  pos = inb(CRT_DATA) << 8;
  outb(CRT_ADDR, CRT_CURSOR_LOW);
  pos |= inb(CRT_DATA);

  crt_pos = pos;
}

static void cga_putc(int c) {
  int attr = (CGA_ATTR_BLACK | CGA_ATTR_LIGHTCYAN) << 8;

  switch (c) {
    case '\n':
      crt_pos += CRT_COLS;
    case '\r':
      crt_pos -= (crt_pos % CRT_COLS);
      break;
    case '\t':
      cga_print("    ");
      break;
    default:
      c |= attr;
      crt_buf[crt_pos++] = c;
      break;
  }

  // move cursor forward
  outb(CRT_ADDR, CRT_CURSOR_HIGH);
  outb(CRT_DATA, crt_pos >> 8);
  outb(CRT_ADDR, CRT_CURSOR_LOW);
  outb(CRT_DATA, crt_pos & 0xff);
}

static void cga_newline() {
  crt_pos = (crt_pos / CRT_COLS + 1) * CRT_COLS;
}

static void cga_print(const char *s) {
  while (*s) {
    cga_putc(*s);
    s++;
  }
}