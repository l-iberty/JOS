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

### 代码解析
#### 缩写
- IMCR = Interrupt Mode Configuration Register
- BSP = Bootstrap Processor
- PIC = Programmable Interrupt Controller (可编程中断控制器)
- APIC = Advanced PIC
- LAPIC = Local APIC
- INTR = Interrupt Request (中断请求)
- NMI = Non-Maskable Interrput (不可屏蔽中断)

#### `kernel/mpconfig.c`
这部分代码由6.828提供，我对照[MP手册](https://pdos.csail.mit.edu/6.828/2018/readings/ia32/MPspec.pdf)进行分析。

**1. MP Floating Pointer Structure**

- 结构体定义：

```c
struct mp {              // floating pointer [MP 4.1]
  uint8_t signature[4];  // "_MP_"
  physaddr_t physaddr;   // phys addr of MP config table
  uint8_t length;        // 1
  uint8_t specrev;       // [14]
  uint8_t checksum;      // all bytes must add up to 0
  uint8_t type;          // MP system config type
  uint8_t imcrp;
  uint8_t reserved[3];
} __attribute__((__packed__));
```

见手册4.1, p39

注意：`imcrp`字段实则包括 MP FEATURE 和 INFORMATION BYTE 2 两部分，前者占 7 bits，后者占 1 bit，总共可用一个`uint8_t`来表示。

- 在内存中搜索 MP Floating Pointer Structure:

```c
// Search for the MP Floating Pointer Structure, which according to
// [MP 4] is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) if there is no EBDA, in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp *mpsearch(void) {
  uint8_t *bda;
  uint32_t p;
  struct mp *mp;

  static_assert(sizeof(*mp) == 16);

  // The BIOS data area lives in 16-bit segment 0x40.
  bda = (uint8_t *)KADDR(0x40 << 4);

  // [MP 4] The 16-bit segment of the EBDA is in the two bytes
  // starting at byte 0x0E of the BDA.  0 if not present.
  if ((p = *(uint16_t *)(bda + 0x0E))) {
    p <<= 4;  // Translate from segment to PA
    if ((mp = mpsearch1(p, 1024))) {
      return mp;
    }
  } else {
    // The size of base memory, in KB is in the two bytes
    // starting at 0x13 of the BDA.
    p = *(uint16_t *)(bda + 0x13) * 1024;
    if ((mp = mpsearch1(p - 1024, 1024))) {
      return mp;
    }
  }
  return mpsearch1(0xF0000, 0x10000);
}
```

手册 p38 指出了 MP Floating Pointer Structure 可能位于哪里：This structure must be stored in at least one of the following memory locations, because the operating system searches for the MP floating pointer structure in the order described below...

- 地址`40:0Eh`处的`uint16_t`是什么？

The exact **starting address of the EBDA segment** for EISA or MCA systems can be found in a two-byte location (40:0Eh) of the BIOS data area.

也就是 EBDA segment 的段地址（段内偏移为0），所以需将其右移4位得到物理地址。然后在 EBDA segment 的开头 1K 内存空间内进行搜索，也就是"**a. In the first kilobyte of Extended BIOS Data Area (EBDA)**"

- 地址`40:13h`处的`uint16_t`是什么？

The BIOS reports the **base memory size** in a two-byte location (40:13h) of the BIOS data area. The base memory size is reported in kilobytes minus 1K, which is used by the EBDA segment or for other purposes.

```c
  p = *(uint16_t *)(bda + 0x13) * 1024;
  if ((mp = mpsearch1(p - 1024, 1024))) {
    return mp;
  }
```
所以上面这段代码的含义是：在 base memory 最后 1K 的内存空间内（e.g., 639K-640K for systems with 640KB of base memory）进行搜索，也就是"**b. Within the last kilobyte of system base memory if the EBDA segment is undefined**"

如果上面两个地方都没有搜索成功，就要到"**c. In the BIOS ROM address space between 0F0000h and 0FFFFFh**"进行搜索。

- `mpsearch1()`实现原理
```c
// Look for an MP structure in the len bytes at physical address addr.
static struct mp *mpsearch1(physaddr_t a, int len) {
  struct mp *mp = KADDR(a), *end = KADDR(a + len);

  for (; mp < end; mp++) {
    if (memcmp(mp->signature, "_MP_", 4) == 0 && sum(mp, sizeof(*mp)) == 0) {
      return mp;
    }
  }
  return NULL;
}
```
首先要确定`signature`是否为`_MP_`，其次还要判断结构体是否有效，见`checksum`的定义：A checksum of the complete pointer structure. All bytes specified by the length field, including CHECKSUM and reserved bytes, must add up to zero.

**2. MP Configuration Table Header**

结构体定义：

```c
struct mpconf {          // configuration table header [MP 4.2]
  uint8_t signature[4];  // "PCMP"
  uint16_t length;       // total table length
  uint8_t version;       // [14]
  uint8_t checksum;      // all bytes must add up to 0
  uint8_t product[20];   // product id
  physaddr_t oemtable;   // OEM table pointer
  uint16_t oemlength;    // OEM table length
  uint16_t entry;        // entry count
  physaddr_t lapicaddr;  // address of local APIC
  uint16_t xlength;      // extended table length
  uint8_t xchecksum;     // extended table checksum
  uint8_t reserved;
  uint8_t entries[0];  // table entries
} __attribute__((__packed__));
```

见手册4.2, p41

和之前一样，这里的`checksum`也是同样的含义：A checksum of the entire base configuration table. All bytes, including CHECKSUM and reserved bytes, must add up to zero.

基于上述分析，就很容易看懂`mpconfig()`了。

**3. MP Configuration Table Entries**
手册4.3, p42-43的内容有助于理解`mp_init()`遍历 entries 的实现逻辑。

**4. Processor Entries**

结构体定义：

```c
struct mpproc {          // processor table entry [MP 4.3.1]
  uint8_t type;          // entry type (0)
  uint8_t apicid;        // local APIC id
  uint8_t version;       // local APIC version
  uint8_t flags;         // CPU flags
  uint8_t signature[4];  // CPU signature
  uint32_t feature;      // feature flags from CPUID instruction
  uint8_t reserved[8];
} __attribute__((__packed__));
```

见手册4.3.1, p43

如果启用多处理器，就会找到多个 Processor Entry，每一个 Processor Entry 都会对应到`cpus`里的一个`struct CpuInfo`，见`mp_init()`。比较特别的一个是 bootstrap processor，对应的`struct CpuInfo`为`bootcpu`所指。

**5. `mp_init()`**

```c
void mp_init(void) {
  ......
  if (mp->imcrp) {
    // [MP 3.2.6.1] If the hardware implements PIC mode,
    // switch to getting interrupts from the LAPIC.
    printf("SMP: Setting IMCR to switch from PIC mode to symmetric I/O mode\n");
    outb(0x22, 0x70);           // Select IMCR
    outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
  }
}
```

回顾`mp::imcrp`的定义：When the IMCR presence bit is set, the IMCR is present and PIC Mode is implemented; otherwise, Virtual Wire Mode is implemented.

<u>实测 JOS 在 qemu 上运行时使用的是 Virtual Wire Mode。</u>

手册3.6.2, p26-27, 三种中断模式：

The MP specification defines three different interrupt modes as follows:

- *PIC Mode* — effectively bypasses all APIC components and forces the system to operate in single-processor mode.
- *Virtual Wire Mode* — uses an APIC as a virtual wire, but otherwise operates the same as PIC Mode.
- *Symmetric I/O Mode* — enables the system to operate with more than one processor.

所以在多处理模式下，需要由 PIC 模式切换到 Symmetric I/O 模式——如果硬件实现了 PIC 模式的话。

下图是 PIC 和 Symmetric I/O 模式下的中断请求传输路径：

![](imgs/pic_mode.png)

![](imgs/symmetric_mode.png)

可以看到，在 PIC 模式下，INTR 经 8259A 直接打到 BSP，NMI 则直接打到 BSP（所以叫不可屏蔽中断）。在 Symmetric I/O 模式下，INTR 不再经过 8259A，而是经过 I/O APIC 打到与每一个 CPU 相连的 Local APIC 上，然后再分别打到每个 CPU 上；NMI 则打到 Local APIC 上。

按照手册p28，由 PIC 切换到 Symmetric I/O 时，首先 write a value of 70h to I/O port 22h, which selects the IMCR. 然后 Writing a value of 01h forces the NMI and 8259 INTR signals to pass through the APIC. 所以`outb(0x23, inb(0x23) | 1)`的作用严格来说不是“屏蔽外部中断”，而是改变 INTR 和 NMI 的路径，如 Figure 3-5 所示。


**小小的意外**

当我把`kernel/mpconfig.c`添加到内核的编译链接里面后，定义在`kernel/pmap.c`的`kern_pgdir`与`kernel/mpconfig.c`里的几个全局变量发生了冲突：它们被分配到了相同的地址！这里*似乎*涉及 BSS 段和 COMMON 段的辨析，参考[https://www.cnblogs.com/tureno/articles/6219494.html](https://www.cnblogs.com/tureno/articles/6219494.html)，但`kern_pgdir`和`kernel/mpconfig.c`里面与它冲突的全局变量都是“未初始化的全局变量”，是不是都应该被放到 COMMON 段里面？另外，我用`objdump -h`或`readelf -S`查看`mpconfig.o`（以及其他 *.o 文件）并未看到 COMMON 段，这是为什么？

解决这个问题有两种途径：

1. gcc 编译选项加上 -fno-common
2. 修改`kernel/kernel.ld`如下：

```
  .bss : {
    PROVIDE(edata = .);
    *(.bss)
    *(COMMON)
    PROVIDE(end = .);
    BYTE(0)
  }
```

两种方式都可行，都是把 COMMON 段的符号塞到 BSS 段里去，目前我采用第1种。6.828采用的是第2种，另外我发现6.828的链接脚本`kern/kernel.ld`也是到了 Lab4 才进行了这样的修改，估计他们也是遇到了同样的问题吧。

#### Per-CPU State and Initialization

- **Per-CPU kernel stack**

6.828要求实现`mem_init_mp()`：

```c
// Modify mappings in kern_pgdir to support SMP
//   - Map the per-CPU stacks in the region [KSTACKTOP-PTSIZE, KSTACKTOP)
//
static void mem_init_mp(void) {
  int i;
  for (i = 0; i < NCPU; i++) {
    boot_map_region(kern_pgdir, KSTACKTOP - i * (KSTKSIZE + KSTKGAP) - KSTKSIZE,
                    KSTKSIZE, PADDR(percpu_kstacks[i]), PTE_W);
  }
}
```

并在`mem_init()`里调用它：

```c
void mem_init() {
  ......
  // Initialize the SMP-related parts of the memory map
  mem_init_mp();
  ......
}
```

现在就可以通过`check_kern_pgdir()`测试了。

- **Per-CPU TSS and TSS descriptor**

修改`trap_init_percpu()`：

```c
// Initialize and load the per-CPU TSS and IDT
void trap_init_percpu(void) {
  int i;
  for (i = 0; i < NCPU; i++) {
    // Setup a TSS so that we get the right stack
    // when we trap to the kernel.
    cpus[i].cpu_ts.ts_esp0 = KSTACKTOP - i * (KSTKSIZE + KSTKGAP);
    cpus[i].cpu_ts.ts_ss0 = GD_KD;
    cpus[i].cpu_ts.ts_iomb = sizeof(struct Taskstate);

    // Initialize the TSS slot of the gdt.
    gdt[(GD_TSS0 >> 3) + i] = SEG16(STS_T32A, (uint32_t)(&cpus[i].cpu_ts),
                                    sizeof(struct Taskstate) - 1, 0);
    gdt[(GD_TSS0 >> 3) + i].sd_s = 0;
  }

  // Load the TSS selector
  ltr(GD_TSS0 + (cpunum() << 3));

  // Load the IDT
  lidt(&idt_pd);
}
```

- **Per-CPU current environment pointer**

原本定义在`kernel/env.c`的全局变量`curenv`要在`kernel/env.h`里将其修改为：
```c
#define curenv (thiscpu->cpu_env)  // Current environment
```

随机而来的是对`tlb_invalidate()`的修改：

```c
void tlb_invalidate(pde_t *pgdir, void *va) {
  // Flush the entry only if we're modifying the current address space.
  // For now, there is only one address space, so always invalidate.
  if (!curenv || curenv->env_pgdir == pgdir) {
    invlpg(va);
  }
}
```

- **Per-CPU system registers**


#### 完成剩余的工作，booting APs

在`i386_init()`里依次调用`lapic_init()`,`pic_init()`和`boot_aps()`，这几个函数的实现原理暂略，直接从6.828抄过来。

`boot_aps()`依次启动每个AP。每个 AP 启动后都会从`0x7000 (MPENTRY_PADDR)`开始执行代码——6.828 Lab4 的**Application Processor Bootstrap**一节对此有详细说明。`kernel/mpentry.asm`是我按照 NASM 语法重写的。需要注意，每个 AP 的入口代码的加载地址都是`MPENTRY_PADDR`，但编译链接后`kernel/mpentry.asm`里面与地址有关的符号都是在 kernel 的链接地址`0xF0100000`之上的。

AP 在执行`kernel/mpentry.asm`代码里的代码时，`eip`始终处于`0x7000 (MPENTRY_PADDR)`之上的低地址，直到`# mov eax, mp_main  # call eax`后，`eip`才打到`KERNBASE`之上。

如何把 LOW EIP 打到`KERNBASE`之上，`kernel/entry.asm`和`mpentry.asm`的做法分别为：

`kernel/entry.asm`:

```
    mov   eax, .relocated
    jmp   eax
.relocated:
```

`kernel/mpentry.asm`:

```
  mov    eax, mp_main
  call   eax
```

二者有一个共同之处：不能直接`jmp/call <label>`，也就是说不能直接`jmp .relocated`/`call mp_main`。如果一定要直接`call`的话务必加上段选择子：

```
  call SELECTOR_FLATC:mp_main
```

这其实就是短跳转和长跳转的区别，不加段选择子是短跳转，加了段选择子就是长跳转，和进入保护模式的那个长跳转是一样的。问题是为什么用寄存器传地址时`jmp/call eax`就是长跳转呢？这应该是指令译码的固有操作。

**另外需要注意**，`mpentry.asm`里的这个地方：

```
  mov    esp, [mpentry_kstack]
```

一定要给`mpentry_kstack`加上`[]`。如果不加`[]`：

```
  mov    esp, mpentry_kstack
```

会造成什么问题？首先对于`mpentry.asm`而言，`mpentry_kstack`是一个从外部导入的符号，在 intel 汇编中不加`[]`就是直接把符号`mpentry_kstack`的值传入`esp`。“符号的值”指的就是`mpentry_kstack`变量的地址，见`obj/kernel/kernel.sym`：

```
f0159230 b env_free_list
f0159234 B mpentry_kstack <--
f0159238 B panicst
```

`mov esp, mpentry_kstack`在编译后就变成`mov esp, 0xf0159234`，这样一来后续的栈操作就改变其他全局变量的内容，例如`env_free_list`，从而造成一些奇怪的BUG。

变量`mpentry_kstack`定义在`kernel/init.c`：

```c
// While boot_aps is booting a given CPU, it communicates the per-core
// stack pointer that should be loaded by mpentry.S to that CPU in
// this variable.
void *mpentry_kstack;
```

`boot_aps()`会修改它以指向不同 APs 的栈顶：

```c
// Start the non-boot (AP) processors.
static void boot_aps(void) {
  ......

  // Boot each AP one at a time
  for (c = cpus; c < cpus + ncpu; c++) {
    if (c == cpus + cpunum())  // We've started already.
      continue;

    // Tell mpentry.S what stack to use
    mpentry_kstack = percpu_kstacks[c - cpus] + KSTKSIZE;

    ......
  }
}
```

这里不会修改变量`mpentry_kstack`的地址(也就是“符号的值”)，只是修改里面的内容。所以在`mpentry.asm`里需要写做`mov esp, [mpentry_kstack]`才能将变量的内容传入`esp`。

**总结：在汇编语言中导入C语言变量时，需要特别注意我们需要的是“符号的值”还是“符号所代表的变量的内容”**

最后，我们成功启动了APs：

<img src="imgs/smp.png" width=700/>

