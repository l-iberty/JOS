# Lab4-1

## Part A: Multiprocessor Support and Cooperative Multitasking

### Multiprocessor Support

#### Exercise 1
Implement `mmio_map_region()` in `kernel/pmap.c`:

```c
//
// Reserve size bytes in the MMIO region and map [pa,pa+size) at this
// location.  Return the base of the reserved region.  size does *not*
// have to be multiple of PGSIZE.
//
void *mmio_map_region(physaddr_t pa, size_t size) {
  // Where to start the next region.  Initially, this is the
  // beginning of the MMIO region.  Because this is static, its
  // value will be preserved between calls to mmio_map_region
  // (just like nextfree in boot_alloc).
  static uintptr_t base = MMIOBASE;

  // Reserve size bytes of virtual memory starting at base and
  // map physical pages [pa,pa+size) to virtual addresses
  // [base,base+size).  Since this is device memory and not
  // regular DRAM, you'll have to tell the CPU that it isn't
  // safe to cache access to this memory.  Luckily, the page
  // tables provide bits for this purpose; simply create the
  // mapping with PTE_PCD|PTE_PWT (cache-disable and
  // write-through) in addition to PTE_W.  (If you're interested
  // in more details on this, see section 10.5 of IA32 volume
  // 3A.)
  //
  // Be sure to round size up to a multiple of PGSIZE and to
  // handle if this reservation would overflow MMIOLIM (it's
  // okay to simply panic if this happens).
  //
  // Hint: The staff solution uses boot_map_region.
  //
  // Your code here:
  uintptr_t res = base;
  size = ROUNDUP(size, PGSIZE);
  if (base + size >= MMIOLIM) {
    panic("mmio_map_region error: reservation would overflow MMIOLIM");
  }
  boot_map_region(kern_pgdir, base, size, pa, PTE_W | PTE_PCD | PTE_PWT);
  base += size;
  return (void *)res;
}
```

#### Exercise 2
... Then modify your implementation of `page_init()` in `kernel/pmap.c` to avoid adding the page at `MPENTRY_PADDR` to the free list, ...

```c
void page_init(void) {
  ...
  for (i = 0, addr = 0; i < npages; i++, addr += PGSIZE) {
    pages[i].pp_ref = 0;

    if (addr == 0 || addr == MPENTRY_PADDR) {
    ...
}
```

#### 对`kernel/pmap.c`测试代码的修改
- `check_page_free_list()`
```c
static void check_page_free_list(bool only_low_memory) {
  ...
  for (pp = page_free_list; pp; pp = pp->pp_link) {
    ...
    // (new test for lab 4)
    assert(page2pa(pp) != MPENTRY_PADDR);
    ...
  }
  ...
}
```

- `check_page()`
```c
static void check_page(void) {
  ...
  // test mmio_map_region
  mm1 = (uintptr_t)mmio_map_region(0, 4097);
  mm2 = (uintptr_t)mmio_map_region(0, 4096);
  // check that they're in the right region
  assert(mm1 >= MMIOBASE && mm1 + 8192 < MMIOLIM);
  assert(mm2 >= MMIOBASE && mm2 + 8192 < MMIOLIM);
  // check that they're page-aligned
  assert(mm1 % PGSIZE == 0 && mm2 % PGSIZE == 0);
  // check that they don't overlap
  assert(mm1 + 8192 <= mm2);
  // check page mappings
  assert(check_va2pa(kern_pgdir, mm1) == 0);
  assert(check_va2pa(kern_pgdir, mm1 + PGSIZE) == PGSIZE);
  assert(check_va2pa(kern_pgdir, mm2) == 0);
  assert(check_va2pa(kern_pgdir, mm2 + PGSIZE) == ~0);
  // check permissions
  assert(*pgdir_walk(kern_pgdir, (void *)mm1, 0) & (PTE_W | PTE_PWT | PTE_PCD));
  assert(!(*pgdir_walk(kern_pgdir, (void *)mm1, 0) & PTE_U));
  // clear the mappings
  *pgdir_walk(kern_pgdir, (void *)mm1, 0) = 0;
  *pgdir_walk(kern_pgdir, (void *)mm1 + PGSIZE, 0) = 0;
  *pgdir_walk(kern_pgdir, (void *)mm2, 0) = 0;

  printf("check_page() succeeded!\n");
}
```

- `check_kern_pgdir()`
```c
static void check_kern_pgdir(void) {
  ...
  // check kernel stack
  // (updated in lab 4 to check per-CPU kernel stacks)
  // for (n = 0; n < NCPU; n++) {
  //   uint32_t base = KSTACKTOP - (KSTKSIZE + KSTKGAP) * (n + 1);
  //   for (i = 0; i < KSTKSIZE; i += PGSIZE) {
  //     assert(check_va2pa(pgdir, base + KSTKGAP + i) == PADDR(percpu_kstacks[n]) + i);
  //   }
  //   for (i = 0; i < KSTKGAP; i += PGSIZE) {
  //     assert(check_va2pa(pgdir, base + i) == ~0);
  //   }
  // }

  // check PDE permissions
  for (i = 0; i < NPDENTRIES; i++) {
    switch (i) {
      case PDX(UVPT):
      case PDX(KSTACKTOP - 1):
      case PDX(UPAGES):
      case PDX(UENVS):
      case PDX(MMIOBASE):
        assert(pgdir[i] & PTE_P);
        break;
      default:
        if (i >= PDX(KERNBASE)) {
          assert(pgdir[i] & PTE_P);
          assert(pgdir[i] & PTE_W);
        } else {
          assert(pgdir[i] == 0);
        }
        break;
    }
  }
  printf("check_kern_pgdir() succeeded!\n");
}
```

(注释尚未打开)

Lab 4 就此启航！
