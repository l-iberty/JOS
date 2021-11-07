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

// lib/syscall.c
void sys_puts(const char *string, size_t len);
int sys_getc(void);
envid_t sys_getenvid(void);
int sys_env_destroy(envid_t);

// lib/libmain.c
extern const char *binaryname;
extern const volatile struct Env *thisenv;

// lib/exit.c
void exit();

// lib/entry_asm.asm
extern const volatile struct Env envs[];
extern const volatile struct PageInfo pages[];

#endif /* JOS_INC_LIB_H */