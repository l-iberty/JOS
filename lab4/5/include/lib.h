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
int sys_getc();
envid_t sys_getenvid();
int sys_env_destroy(envid_t);
void sys_yield();
envid_t sys_exofork();
int sys_env_set_status(envid_t envid, int status);
int sys_env_set_pgfault_upcall(envid_t envid, void *func);
int sys_page_alloc(envid_t envid, void *va, int perm);
int sys_page_map(envid_t srcenvid, void *srcva, envid_t dstenvid, void *dstva, int perm);
int sys_page_unmap(envid_t envid, void *va);
int sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm);
int sys_ipc_recv(void *dstva);

// lib/pgfault.c
void set_pgfault_handler(void (*handler)(struct UTrapframe *utf));

// lib/libmain.c
extern const char *binaryname;
extern const volatile struct Env *thisenv;

// lib/exit.c
void exit();

// lib/fork.c
#define PTE_SHARE 0x400
envid_t fork(void);
envid_t sfork(void);  // Challenge!

// ipc.c
void ipc_send(envid_t to_env, uint32_t value, void *pg, int perm);
int32_t ipc_recv(envid_t *from_env_store, void *pg, int *perm_store);
envid_t ipc_find_env(enum EnvType type);

// lib/entry_asm.asm
extern const volatile struct Env envs[];
extern const volatile struct PageInfo pages[];

#endif /* JOS_INC_LIB_H */