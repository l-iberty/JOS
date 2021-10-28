#include <include/env.h>
#include <include/syscall.h>
#include <include/types.h>

int32_t _syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

void sys_puts(const char *s, size_t len) {
  _syscall(SYS_puts, (uint32_t)s, len, 0, 0, 0);
}

int sys_getc() {
  return _syscall(SYS_getc, 0, 0, 0, 0, 0);
}

int sys_env_destory(envid_t envid) {
  return _syscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}

envid_t sys_getenvid() {
  return _syscall(SYS_getenvid, 0, 0, 0, 0, 0);
}