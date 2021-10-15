#ifndef _JOS_STDIO_H_
#define _JOS_STDIO_H_

#include <include/types.h>

// lib/vsprintf.c
void vsprintf(char *buf, const char *fmt, va_list ap);

// lib/printf.c
void printf(const char *fmt, ...);
void vprintf(const char *fmt, va_list ap);

// lib/readline.c
char *readline(const char *prompt);

// kernel/console.c
void putchar(int c);
int getchar();
int iscons(int fdnum);


#endif /* _JOS_STDIO_H_ */