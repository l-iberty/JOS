#include <include/string.h>
#include <include/memlayout.h>
#include <include/lib.h>
#include <include/kbdreg.h>

#include <kernel/console.h>

/***** Text-mode CGA display output *****/

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
    case '\b':
      if (crt_pos > 0) {
        crt_pos--;
        crt_buf[crt_pos] = ' ';
      }
      break;
    case '\n':
      crt_pos += CRT_COLS;
    case '\r':
      crt_pos -= (crt_pos % CRT_COLS);
      break;
    case '\t':
      cga_putc(' ');
      cga_putc(' ');
      cga_putc(' ');
      cga_putc(' ');
      break;
    default:
      c |= attr;
      crt_buf[crt_pos++] = c;
      break;
  }

  // What is the purpose of this?
  // 当显示范围超出当前屏幕大小时进行“滚屏”
  if (crt_pos >= CRT_SIZE) {
    int i;

    memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
		for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++) {
      // crt_buf[i] = 0x0700 | ' '; // 黑底(00)灰色(07)的空格' '. 颜色属性不重要, 只要是空格' '就行了.
      crt_buf[i] = ' ';
    }
    crt_pos -= CRT_COLS;
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

/***** Keyboard input code *****/

#define NO    0

#define SHIFT (1<<0)
#define CTL   (1<<1)
#define ALT   (1<<2)

#define CAPSLOCK    (1<<3)
#define NUMLOCK     (1<<4)
#define SCROLLLOCK  (1<<5)

#define E0ESC       (1<<6)

static uint8_t shiftcode[256] = {
  [0x1D] = CTL,
  [0x2A] = SHIFT,
  [0x36] = SHIFT,
  [0x38] = ALT,
  [0x9D] = CTL,
  [0xB8] = ALT
};

static uint8_t togglecode[256] = {
  [0x3A] = CAPSLOCK,
  [0x45] = NUMLOCK,
  [0x46] = SCROLLLOCK
};

static uint8_t normalmap[256] = {
  NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',	// 0x00
  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',	// 0x10
  'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',	// 0x20
  '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
  'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',	// 0x30
  NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
  NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',	// 0x40
  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
  '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,	// 0x50
  [0xC7] = KEY_HOME,	      [0x9C] = '\n' /*KP_Enter*/,
  [0xB5] = '/' /*KP_Div*/,      [0xC8] = KEY_UP,
  [0xC9] = KEY_PGUP,	      [0xCB] = KEY_LF,
  [0xCD] = KEY_RT,	      [0xCF] = KEY_END,
  [0xD0] = KEY_DN,	      [0xD1] = KEY_PGDN,
  [0xD2] = KEY_INS,	      [0xD3] = KEY_DEL
};

static uint8_t shiftmap[256] = {
  NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',	// 0x00
  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',	// 0x10
  'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',	// 0x20
  '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
  'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',	// 0x30
  NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
  NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',	// 0x40
  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
  '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,	// 0x50
  [0xC7] = KEY_HOME,            [0x9C] = '\n' /*KP_Enter*/,
  [0xB5] = '/' /*KP_Div*/,      [0xC8] = KEY_UP,
  [0xC9] = KEY_PGUP,      [0xCB] = KEY_LF,
  [0xCD] = KEY_RT,	      [0xCF] = KEY_END,
  [0xD0] = KEY_DN,	      [0xD1] = KEY_PGDN,
  [0xD2] = KEY_INS,	      [0xD3] = KEY_DEL
};

#define C(x) (x - '@')

static uint8_t ctlmap[256] = {
  NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
  NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
  C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
  C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
  C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
  NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
  C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
  [0x97] = KEY_HOME,
  [0xB5] = C('/'),      [0xC8] = KEY_UP,
  [0xC9] = KEY_PGUP,    [0xCB] = KEY_LF,
  [0xCD] = KEY_RT,      [0xCF] = KEY_END,
  [0xD0] = KEY_DN,      [0xD1] = KEY_PGDN,
  [0xD2] = KEY_INS,     [0xD3] = KEY_DEL
};

static uint8_t *charcode[4] = {
  normalmap,
  shiftmap,
  ctlmap,
  ctlmap
};

static void cons_intr(int (*proc)());

/*
 * Get data from the keyboard.  If we finish a character, return it.  Else 0.
 * Return -1 if no data.
 */
static int kbd_proc_data(void) {
  int c;
  uint8_t stat, data;
  static uint32_t shift;

  stat = inb(KBSTATP);
  if ((stat & KBS_DIB) == 0)
    return -1;
  // Ignore data from mouse.
  if (stat & KBS_TERR)
    return -1;

  data = inb(KBDATAP);

  if (data == 0xE0) {
    // E0 escape character
    shift |= E0ESC;
    return 0;
  } else if (data & 0x80) {
    // Key released
    data = (shift & E0ESC ? data : data & 0x7F);
    shift &= ~(shiftcode[data] | E0ESC);
    return 0;
  } else if (shift & E0ESC) {
    // Last character was an E0 escape; or with 0x80
    data |= 0x80;
    shift &= ~E0ESC;
  }

  shift |= shiftcode[data];
  shift ^= togglecode[data];

  c = charcode[shift & (CTL | SHIFT)][data];
  if (shift & CAPSLOCK) {
    if ('a' <= c && c <= 'z') {
      c += 'A' - 'a';
    } else if ('A' <= c && c <= 'Z') {
      c += 'a' - 'A';
    }
  }

  return c;
}

static void kbd_intr(void) { cons_intr(kbd_proc_data); }

static void kbd_init(void) {}

/***** General device-independent console code *****/
// Here we manage the console input buffer,
// where we stash characters received from the keyboard or serial port
// whenever the corresponding interrupt occurs.

#define CONSBUFSIZE 512

static struct {
  uint8_t buf[CONSBUFSIZE];
  uint32_t rpos;
  uint32_t wpos;
} cons;

// called by device interrupt routines to feed input characters
// into the circular console input buffer.
static void cons_intr(int (*proc)(void)) {
  int c;

  while ((c = (*proc)()) != -1) {
    if (c == 0)
      continue;
    cons.buf[cons.wpos++] = c;
    if (cons.wpos == CONSBUFSIZE)
      cons.wpos = 0;
  }
}

// return the next input character from the console, or 0 if none waiting
int cons_getc(void) {
  int c;

  // poll for any pending input characters,
  // so that this function works even when interrupts are disabled
  // (e.g., when called from the kernel monitor).
  kbd_intr();

  // grab the next character from the input buffer.
  if (cons.rpos != cons.wpos) {
    c = cons.buf[cons.rpos++];
    if (cons.rpos == CONSBUFSIZE)
      cons.rpos = 0;
    return c;
  }
  return 0;
}

void cons_init() {
  cga_init();
  kbd_init();
}

void cons_print(const char *s) {
  cga_print(s);
}

void cons_putc(int c) {
  cga_putc(c);
}

// `High'-level console I/O.  Used by readline.

void putchar(int c) { cons_putc(c); }

int getchar() {
  int c;

  while ((c = cons_getc()) == 0)
    /* do nothing */;
  return c;
}

int iscons(int fdnum) {
  // used by readline
  return 1;
}