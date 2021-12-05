// implement fork from user space

#include <include/lib.h>
#include <include/string.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void pgfault(struct UTrapframe *utf) {
  void *addr = (void *)utf->utf_fault_va;
  uint32_t err = utf->utf_err;
  int r;

  // Check that the faulting access was (1) a write, and (2) to a
  // copy-on-write page.  If not, panic.
  // Hint:
  //   Use the read-only page table mappings at uvpt
  //   (see <inc/memlayout.h>).

  // LAB 4: Your code here.

  printf("pafault() invoked. addr: %08x, err: 0x%x, eip: %08x\n", addr, err, utf->utf_eip);

  if ((err & FEC_WR) == 0 || (uvpt[PGNUM(addr)] & PTE_COW) == 0) {
    panic("faulting access\n           fault_va: %08x, err: 0x%x, pte: %03x, eip: %08x", utf->utf_fault_va, err,
          uvpt[PGNUM(addr)], utf->utf_eip);
  }

  // Allocate a new page, map it at a temporary location (PFTEMP),
  // copy the data from the old page to the new page, then move the new
  // page to the old page's address.
  // Hint:
  //   You should make three system calls.

  // LAB 4: Your code here.

  addr = ROUNDDOWN(addr, PGSIZE);

  // Allocates a new page mapped at a temporary location and copies the
  // contents of the faulting page into it.
  if ((r = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W)) < 0) {
    panic("sys_page_alloc");
  }
  memmove(PFTEMP, addr, PGSIZE);

  // Maps the new page at the appropriate address with read/write permissions,
  // in place of the old read-only mapping.
  if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_P | PTE_U | PTE_W)) < 0) {
    panic("sys_page_map");
  }
  if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
    panic("sys_page_unmap");
  }
}

//
// Map our virtual page (address addr) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int duppage(envid_t envid, void *addr) {
  int r;

  // LAB 4: Your code here.

  // Map the page copy-on-write into the address space of the child and
  // then remap the page copy-on-write in its own address space.

  if (!PAGE_ALGINED(addr)) {
    panic("addr %08x not page-aligned", addr);
  }

  pte_t pte = uvpt[PGNUM(addr)];
  int perm = PTE_P | PTE_U;

  if ((pte & PTE_W) || (pte & PTE_COW)) {
    perm |= PTE_COW;
  }

  if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0) {
    panic("sys_page_map");
  }

  if ((r = sys_page_map(0, addr, 0, addr, (pte & PTE_SYSCALL) | PTE_COW)) < 0) {
    panic("sys_page_map");
  }

  return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t fork(void) {
  // LAB 4: Your code here.

  envid_t envid;
  uintptr_t addr;
  int r;
  extern unsigned char end[];

  // The parent installs pgfault() as the C-level page fault handler,
  // using the set_pgfault_handler() function you implemented above.
  set_pgfault_handler(pgfault);

  // The parent calls sys_exofork() to create a child environment.
  envid = sys_exofork();
  if (envid < 0) {
    panic("sys_exofork");
  }
  if (envid == 0) {
    // We're the child.
    // The copied value of the global variable 'thisenv'
    // is no longer valid (it refers to the parent!).
    // Fix it and return 0.
    printf("child envid %08x\n", sys_getenvid());
    thisenv = &envs[ENVX(sys_getenvid())];
    return 0;
  }

  // We're the parent.

  // For each writable or copy-on-write page in its address space below UTOP,
  // the parent calls duppage(), which should map the page copy-on-write
  // into the address space of the child and then remap the page copy-on-write
  // in its own address space.
  for (addr = UTEXT; addr < UTOP; addr += PGSIZE) {
    if (addr == (uintptr_t)(UXSTACKTOP - PGSIZE)) {
      continue;
    }
    if ((uvpd[PDX(addr)] & PTE_P) == 0) {
      continue;
    }
    if (uvpt[PGNUM(addr)] & PTE_P) {
      duppage(envid, (void *)addr);
    }
  }

  // The parent sets the user page fault entrypoint for the child to
  // look like its own.
  if ((r = sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall)) < 0) {
    panic("sys_env_set_pgfault_upcall");
  }

  // Allocate a fresh page in the child for the exception stack.
  if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) < 0) {
    panic("sys_page_alloc");
  }

  // The child is now ready to run, so the parent marks it runnable.
  if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
    panic("sys_env_set_status");
  }

  return envid;
}

// Challenge!
int sfork(void) {
  panic("sfork not implemented");
  return -E_INVAL;
}
