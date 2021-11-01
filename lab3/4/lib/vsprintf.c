#include <include/stdarg.h>
#include <include/string.h>
#include <include/types.h>

/**
 * convert integer to string
 *
 * @param val
 * @param base  16 for hexadecimal, 10 for decimal, etc.
 * @param ps    ptr to string buffer
 *
 * @return the resulting string
 */
static char *itoa(unsigned int val, int base, char **ps) {
  char *res = *ps;
  unsigned int m = val % base;
  unsigned int n = val / base;
  if (n) {
    itoa(n, base, ps);
  }
  *(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');
  *(*ps) = 0; /* end of string */

  return res;
}

void vsprintf(char *buf, const char *fmt, va_list ap) {
  char *p, *q, *s, padc, ch;
  char tmp_buf[10];
  int v, base, precision, width, sn;

  for (p = buf; *fmt; fmt++) {
    if (*fmt == '%') {
      s = tmp_buf;
      padc = ' ';
      precision = width = 0;

    reswitch:
      switch (*++fmt) {
        case 'd':
          base = 10;
          goto number;
        case 'o':
          base = 8;
          goto number;
        case 'p':
          strcpy(p, "0x");
          p += 2;
        case 'x':
          base = 16;
        number:
          v = va_arg(ap, unsigned int);
          s = itoa(v, base, &s);
          sn = strlen(s);
          width = sn;
          for (; precision > width; width++) {
            *p++ = padc;
          }
          strcpy(p, s);
          p += sn;
          break;
        case 's':
          q = va_arg(ap, char *);
          if (q == NULL) {
            q = "(null)";
          }
          width = strlen(q);
          if (precision > 0 && precision < width) {
            width = precision;
          }
          strncpy(p, q, width);
          p += width;
          break;

        case '.':
          width = 0;
          goto reswitch;
        case '*':
          precision = va_arg(ap, int);
          goto reswitch;

        // flag to pad with 0's instead of space
        case '0':
          padc = '0';
          goto reswitch;

        // width field
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          for (precision = 0;; ++fmt) {
            ch = *fmt;
            if (ch < '0' || ch > '9') {
              break;
            }
            precision = (precision * 10) + ch - '0';
          }
          fmt--;
          goto reswitch;
      }
    } else {
      *p++ = *fmt;
    }
  }
}
