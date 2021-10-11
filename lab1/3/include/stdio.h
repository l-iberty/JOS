#ifndef _JOS_STDIO_H_
#define _JOS_STDIO_H_

#include <include/types.h>

void vsprintf(char *buf, const char *fmt, va_list ap);

void printf(const char *fmt, ...);
void vprintf(const char *fmt, va_list ap);

#endif /* _JOS_STDIO_H_ */