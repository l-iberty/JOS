# Lab2-1

## 使用 vscode-gdb 调试代码
1. 在目标机上安装插件“Native Debug”
2. "Run -> Add Configuration... -> GDB", 编辑`launch.json`:
```json
{
	// Use IntelliSense to learn about possible attributes.
	// Hover to view descriptions of existing attributes.
	// For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
	"version": "0.2.0",
	"configurations": [
		{
			"name": "Debug",
			"type": "gdb",
			"request": "attach",
			"remote": true,
			"target": "localhost:1234",
			"cwd": "${workspaceRoot}",
			"executable": "${workspaceRoot}/lab2/1/obj/kernel/kernel",
			"valuesFormatting": "parseText"
		}
	]
}
```
3. `make qemu-gdb`准备好后，在 vscode 里`F5`便可以开启调试并命中断点：
![](imgs/vscode-gdb.png)

但是由于未知原因，有的变量无法在`VARIABLES`窗口查看，还是要在`DEBUG CONSOLE`里使用gdb命令进行操作。

## Getting started
Lab2 Memory Management 需要新增以下几个文件：
```
kernel
├── kclock.c
├── kclock.h
├── pmap.c
└── pmap.h
```
从6.828拷贝过来即可。`kernel/init.c`的`i386_init()`将会调用`kernel/pmap.c`的`mem_init()`，开启 Lab2 之旅。

## Implementations
### 物理内存探测: `i386_detect_memory()`
```c
static void i386_detect_memory(void) {
  size_t basemem, extmem, ext16mem, totalmem;

  // Use CMOS calls to measure available base & extended memory.
  // (CMOS calls return results in kilobytes.)
  basemem = nvram_read(NVRAM_BASELO);
  extmem = nvram_read(NVRAM_EXTLO);
  ext16mem = nvram_read(NVRAM_EXT16LO) * 64;

  // Calculate the number of physical pages available in both base
  // and extended memory.
  if (ext16mem) {
    totalmem = 16 * 1024 + ext16mem;
  } else if (extmem) {
    totalmem = 1 * 1024 + extmem;
  } else {
    totalmem = basemem;
  }

  npages = totalmem / (PGSIZE / 1024);
  npages_basemem = basemem / (PGSIZE / 1024);

  printf("Physical memory: %dK available, base = %dK, extended = %dK\n", totalmem, basemem, totalmem - basemem);
}
```
通过 MC146818 REAL-TIME CLOCK PLUS RAM (RTC) 获取内存信息，分为三段：
- `basemem`：`[0,1M)`范围内的可用内存
- `extmem`：`[1M,16M)`范围内的可用内存
- `ext16mem`：`[16M,4G)`范围内的可用内存

（附上 MC146818 芯片[资料](https://pdf1.alldatasheet.com/datasheet-pdf/view/122156/MOTOROLA/MC146818.html)）

结果如下图所示：

![](imgs/mem_detect.png)

物理内存总量为128M。基本内存总量为640K。这里的“640K基本内存、1M以上的扩展内存”是什么？

DOS操作系统最早设计时，8086 CPU只支持1M的寻址空间，所以DOS只能管理最多1M字节的连续内存空间。在这1M内存中，又只有640K被留给应用程序使用，它们被称为基本内存，其它384K被称为高端内存，是留给视频显示和BIOS等使用的。现在的PC机仍需要与DOS兼容，所以保留了当年的640K基本内存这种结构。

到了80286时代，保护模式出现了。80286 CPU拥有24位地址总线，通过其保护模式提供了16M的内存寻址空间。后面的 80386 CPU 拥有32位地址总线，寻址空间达到了4G。

### `mem_init()`
1. 调用`i386_detect_memory()`探测物理内存，初始化全局变量`npages`和`npages_basemem`。

2. 在 kernel 的`.bss`段后面分配一个page用做页目录：
```c
  // create initial page directory.
  kern_pgdir = (pde_t *)boot_alloc(PGSIZE);
  memset(kern_pgdir, 0, PGSIZE);
```
注意`kern_pgdir`是一个虚拟地址，位于`KERNBASE`之上。

3. 把虚拟地址`UVPT`映射到`kern_pgdir`所在的物理页，暂时不知其用意：
```c
  // Recursively insert PD in itself as a page table, to form
  // a virtual page table at virtual address UVPT.
  // (For now, you don't have understand the greater purpose of the
  // following line.)

  // Permissions: kernel R, user R
  kern_pgdir[PDX(UVPT)] = PADDR(kern_pgdir) | PTE_U | PTE_P;
```

4. 分配`npages`个连续的`PageInfo`用于管理全部的物理页：
```c
  // Allocate an array of npages 'struct PageInfo's and store it in 'pages'.
  // The kernel uses this array to keep track of physical pages: for
  // each physical page, there is a corresponding struct PageInfo in this
  // array.  'npages' is the number of physical pages in memory.  Use memset
  // to initialize all fields of each struct PageInfo to 0.
  // Your code goes here:
  pages = (struct PageInfo *)boot_alloc(npages * sizeof(struct PageInfo));
  memset(pages, 0, npages * sizeof(struct PageInfo));
```

![](imgs/pages.png)

上图中的每个矩形代表一个`PageInfo`，里面的数字是该`PageInfo`对应的物理页地址。`kernel/pmap.h`提供了一个由给定的`PageInfo`得到对应物理页地址的函数`page2pa()`:
```c
static inline physaddr_t page2pa(struct PageInfo *pp) {
  return (pp - pages) << PGSHIFT;
}
```