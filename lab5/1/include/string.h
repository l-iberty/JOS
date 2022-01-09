#ifndef JOS_INC_STRING_H
#define JOS_INC_STRING_H

#include <include/types.h>

int strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t size);
int strcmp(const char *p, const char *q);
int strncmp(const char *p, const char *q, size_t n);
char *strchr(const char *s, char c);

void memset(void *dst, char ch, int n);
void *memmove(void *dst, const void *src, size_t n);
void *memcpy(void *dst, const void *src, int n);
int memcmp(const void *v1, const void *v2, size_t n);

long strtol(const char *s, char **endptr, int base);

#endif /* JOS_INC_STRING_H */