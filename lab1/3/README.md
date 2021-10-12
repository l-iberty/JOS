# Lab1-3

## 实现`printf`


## virtual memory
与6.828一样，建立一个页表，把虚拟地址`[KERNBASE, KERNBASE+4MB)`映射到物理地址`[0,4MB)`，见`kernel/entrypgdir.c`：
```c
// The entry.S page directory maps the first 4MB of physical memory
// starting at virtual address KERNBASE (that is, it maps virtual
// addresses [KERNBASE, KERNBASE+4MB) to physical addresses [0, 4MB)).
// We choose 4MB because that's how much we can map with one page
// table and it's enough to get us through early boot.  We also map
// virtual addresses [0, 4MB) to physical addresses [0, 4MB); this
// region is critical for a few instructions in entry.S and then we
// never use it again.
//
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.  Also, because of restrictions
// related to linking and static initializers, we use "x + PTE_P"
// here, rather than the more standard "x | PTE_P".  Everywhere else
// you should use "|" to combine flags
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

要点：
1. 为什么是“4MB”？
因为一个页表只能映射这么多，到目前为止4MB内存已经够用了。
2. 设置属性位时为什么使用`x + PTE_P`，而不是更标准的写法`x + PTE_P`？
因为编译器不让：
```
kernel/entrypgdir.c:24:5: error: initializer element is not constant
   = ((uintptr_t)entry_pgtable - KERNBASE) | PTE_P,
     ^
kernel/entrypgdir.c:24:5: note: (near initialization for ‘entry_pgdir[0]’)
kernel/entrypgdir.c:27:5: error: initializer element is not constant
   = ((uintptr_t)entry_pgtable - KERNBASE) | PTE_P | PTE_W
     ^
kernel/entrypgdir.c:27:5: note: (near initialization for ‘entry_pgdir[960]’)
kernel/Makefile:28: recipe for target 'obj/kernel/entrypgdir.o' failed
```
3. `[0,4MB)`这块内存被设置为只读，如果开启分页，任何代码都禁止向这块内存写入，包括ring0。原因是`kernel/entry.asm`通过`cr0`开启了“写保护”。这就导致了一个问题，如何向CGA显存空间`[0xB0000, 0xBFFFF]`写数据，`printf`不能工作了吗？解决方法是将虚拟地址加上`KERNBASE`。虚拟内存`[KERNBASE,KERNBASE+4MB)`是可写的，而它映射到物理内存`[0,4MB)`。所以`kernel/console.c`的`cga_init()`这样初始化`crt_buf`：
```c
crt_buf = (uint16_t *)(CGA_BASE + KERNBASE);
```

## kernel