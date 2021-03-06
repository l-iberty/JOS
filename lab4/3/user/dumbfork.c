// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two, crudely.

#include <include/lib.h>
#include <include/string.h>

envid_t dumbfork(void);

void umain(int argc, char **argv) {
  envid_t who;
  int i;

  // fork a child process
  who = dumbfork();

  // print a message and yield to the other a few times
  for (i = 0; i < (who ? 2 : 4); i++) {
    printf("%d: I am the %s!\n", i, who ? "parent" : "child");
    sys_yield();
  }
}

void duppage(envid_t dstenv, void *addr) {
  int r;

  // This is NOT what you should do in your fork.
  // 在 child 地址空间的 addr 地址上分配一个 page.
  if ((r = sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W)) < 0) {
    panic("sys_page_alloc: %e", r);
  }
  // 把 child 在虚拟地址 addr 上的 page 映射到 parent 地址空间的 UTMP 地址上.
  if ((r = sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W)) < 0) {
    panic("sys_page_map: %e", r);
  }
  // 虚拟地址 addr 在 child & parent 地址空间里都是有效的, 但现在还未切换 cr3,
  // 所以 memmove 操作的还是 parent 的地址空间. memmove 在 parent 地址空间里把
  // 虚拟地址 addr 的内存拷贝到虚拟地址 UTEMP, 于是 child 就可以在自身地址空间的
  // addr 地址上看到和 parent 相同的内容.
  memmove(UTEMP, addr, PGSIZE);

  // 解除 parent 在虚拟地址 UTEMP 上的映射.
  if ((r = sys_page_unmap(0, UTEMP)) < 0) {
    panic("sys_page_unmap: %e", r);
  }
}

envid_t dumbfork(void) {
  envid_t envid;
  uint8_t *addr;
  int r;
  extern unsigned char end[];

  // Allocate a new child environment.
  // The kernel will initialize it with a copy of our register state,
  // so that the child will appear to have called sys_exofork() too -
  // except that in the child, this "fake" call to sys_exofork()
  // will return 0 instead of the envid of the child.
  envid = sys_exofork();
  if (envid < 0) {
    panic("sys_exofork: %e", envid);
  }
  if (envid == 0) {
    // We're the child.
    // The copied value of the global variable 'thisenv'
    // is no longer valid (it refers to the parent!).
    // Fix it and return 0.
    thisenv = &envs[ENVX(sys_getenvid())];
    return 0;
  }

  // We're the parent.
  // Eagerly copy our entire address space into the child.
  // This is NOT what you should do in your fork implementation.
  for (addr = (uint8_t *)UTEXT; addr < end; addr += PGSIZE) {
    duppage(envid, addr);
  }

  // Also copy the stack we are currently running on.
  // addr 是分配在栈上的变量, user environment 的栈只有 PGSIZE 这么大 (见 include/memlayout.h ).
  // 所以把 &addr 向下对齐 PGSIZE 就能够得到栈底的虚拟地址 (USTACKTOP - PGSIZE).
  duppage(envid, ROUNDDOWN(&addr, PGSIZE));

  // Start the child environment running
  if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
    panic("sys_env_set_status: %e", r);
  }

  return envid;
}
