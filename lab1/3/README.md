# Lab1-3

## 实现`printf`


## virtual memory
与6.828一样，建立一个页表，把虚拟地址`[KERNBASE, KERNBASE+4MB)`映射到物理地址`[0,4MB)`，见`kernel/entrypgdir.c`：
```c
__attribute__((__aligned__(PGSIZE)))
pde_t entry_pgdir[NPDENTRIES] = {
    // Map VA's [0, 4MB) to PA's [0, 4MB)
    [0]
        = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P,
    // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
    [KERNBASE>>PDXSHIFT]
        = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P + PTE_W
};
```

## kernel