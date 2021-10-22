#ifndef JOS_INC_LIB_H
#define JOS_INC_LIB_H

#include <include/assert.h>
#include <include/env.h>
#include <include/error.h>
#include <include/memlayout.h>
#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/string.h>
#include <include/syscall.h>
#include <include/types.h>

#define USED(x) (void)(x)

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t byte);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t word);
uint32_t ind(uint16_t port);
void outd(uint16_t port, uint32_t dword);
void insb(uint16_t port, void *addr, int cnt);
void insw(uint16_t port, void *addr, int cnt);
void insd(uint16_t port, void *addr, int cnt);

uint32_t rcr0();
uint32_t rcr3();
void lcr0(uint32_t x);
void lcr3(uint32_t x);

void invlpg(void *addr);

// main user program
void umain(int argc, char **argv);

// libmain.c or entry.S
extern const char *binaryname;
extern const volatile struct Env *thisenv;
extern const volatile struct Env envs[NENV];
extern const volatile struct PageInfo pages[];

// exit.c
void exit(void);

// readline.c
char *readline(const char *buf);

// syscall.c
void sys_cputs(const char *string, size_t len);
int sys_cgetc(void);
envid_t sys_getenvid(void);
int sys_env_destroy(envid_t);

/* File open modes */
#define O_RDONLY 0x0000  /* open for reading only */
#define O_WRONLY 0x0001  /* open for writing only */
#define O_RDWR 0x0002    /* open for reading and writing */
#define O_ACCMODE 0x0003 /* mask for above modes */

#define O_CREAT 0x0100 /* create if nonexistent */
#define O_TRUNC 0x0200 /* truncate to zero length */
#define O_EXCL 0x0400  /* error if already exists */
#define O_MKDIR 0x0800 /* create directory, not regular file */

#endif /* JOS_INC_LIB_H */