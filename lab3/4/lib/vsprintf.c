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
  char *p, *q, *s;
  char tmp_buf[10];
  int v, precision, width;

  for (p = buf; *fmt; fmt++) {
    if (*fmt == '%') {
      s = tmp_buf;
      precision = 0;

    reswitch:
      switch (*++fmt) {
        case 'd':
          v = va_arg(ap, int);
          s = itoa(v, 10, &s);
          strcpy(p, s);
          p += strlen(s);
          break;
        case 'o':
          v = va_arg(ap, int);
          s = itoa(v, 8, &s);
          strcpy(p, s);
          p += strlen(s);
          break;
        case 'p':
          strcpy(p, "0x");
          p += 2;
        case 'x':
          v = va_arg(ap, unsigned int);
          s = itoa(v, 16, &s);
          strcpy(p, s);
          p += strlen(s);
          break;
        case 's':
          q = va_arg(ap, char *);
          if (q == NULL) {
            q = "(null)";
          }
          width = strlen(q);
          if (precision > 0 && precision <= width) {
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
      }
    } else {
      *p++ = *fmt;
    }
  }
}
