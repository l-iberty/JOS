#ifndef _JOS_STRING_H_
#define _JOS_STRING_H_

#include <include/types.h>

int strlen(const char *s);
char *strcpy(char *dst, const char *src);
int strcmp(const char *p, const char *q);
int strncmp(const char *p, const char *q, size_t n);
char *strchr(const char *s, char c);

void memset(void *dst, char ch, int n);
void *memmove(void *dst, const void *src, size_t n);

#endif /* _JOS_STRING_H_ */