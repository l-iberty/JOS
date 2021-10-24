#ifndef JOS_INC_STDIO_H
#define JOS_INC_STDIO_H

#include <include/types.h>

// lib/vsprintf.c
void vsprintf(char *buf, const char *fmt, va_list ap);

// kernel/kprintf.c
void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list ap);

// lib/readline.c
char *readline(const char *prompt);

// kernel/console.c
void putchar(int c);
int getchar();
int iscons(int fdnum);

#endif /* JOS_INC_STDIO_H */