#include <include/env.h>
#include <include/syscall.h>
#include <include/types.h>

int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

void sys_puts(const char *s, size_t len) {
  syscall(SYS_puts, (uint32_t)s, len, 0, 0, 0);
}

int sys_getc() {
  return syscall(SYS_getc, 0, 0, 0, 0, 0);
}

int sys_env_destroy(envid_t envid) {
  return syscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}

envid_t sys_getenvid() {
  return syscall(SYS_getenvid, 0, 0, 0, 0, 0);
}

int sys_yield() {
  return syscall(SYS_yield, 0, 0, 0, 0, 0);
}

envid_t sys_exofork() {
  return syscall(SYS_exofork, 0, 0, 0, 0, 0);
}

int sys_env_set_status(envid_t envid, int status) {
  return syscall(SYS_env_set_status, envid, status, 0, 0, 0);
}

int sys_env_set_pgfault_upcall(envid_t envid, void *func) {
  return syscall(SYS_env_set_pgfault_upcall, envid, (uint32_t)func, 0, 0, 0);
}

int sys_page_alloc(envid_t envid, void *va, int perm) {
  return syscall(SYS_page_alloc, envid, (uint32_t)va, perm, 0, 0);
}

int sys_page_map(envid_t srcenvid, void *srcva, envid_t dstenvid, void *dstva, int perm) {
  return syscall(SYS_page_map, srcenvid, (uint32_t)srcva, dstenvid, (uint32_t)dstva, perm);
}

int sys_page_unmap(envid_t envid, void *va) {
  return syscall(SYS_page_unmap, envid, (uint32_t)va, 0, 0, 0);
}

int sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm) {
  return syscall(SYS_ipc_try_send, envid, value, (uint32_t)srcva, perm, 0);
}

int sys_ipc_recv(void *dstva) {
  return syscall(SYS_ipc_recv, (uint32_t)dstva, 0, 0, 0, 0);
}