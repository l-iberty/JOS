# Lab3-2

## Part A: User Environments and Exception Handling

先重写一下`Makefile`。以`kernel/Makefile`为例：
```Makefile
KERNEL_SRCS := kernel/console.c \
	kernel/entry.asm \
	kernel/entrypgdir.c \
	kernel/env.c \
	kernel/init.c \
	kernel/kclock.c \
	kernel/lib.asm \
	kernel/monitor.c \
	kernel/pmap.c \
	kernel/trap.c

KERNEL_OBJS := $(patsubst kernel/%.c, $(OBJDIR)/kernel/%.o, $(KERNEL_SRCS))
KERNEL_OBJS := $(patsubst kernel/%.asm, $(OBJDIR)/kernel/%.o, $(KERNEL_OBJS))

$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + cc $@
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/%.o: kernel/%.asm
	@echo + cc $@
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) $< -o $@
```

像过去那样每增加一个源文件就要修改`Makefile`很是麻烦，所以向6.828学习，使用上面那样的写法。先解释`KERNEL_OBJS`的生成原理：

`patsubst`是模式替换函数。

- 格式：`$(patsubst <pattern>, <replacement>, <text>)`
- 功能：查找`<text>`中的字符串，把符合模式`<pattern>`的字符串替换为`<replacement>`。这里，`<pattern>`可以包括通配符`%`，表示任意长度的字符串。如果`<replacement>`中也包含`%`，那么`<replacement>`中的这个`%`将是`<pattern>`中的那个`%`所代表的字符串。

`KERNEL_OBJS`来自两部分，一是`*.c`文件，二是`*.asm`文件。首先把`KERNEL_SRCS`中的`*.c`文件替换为相应的`*.o`文件，注意，不匹配的部分（也就是`*.asm`文件）会被原样赋值给`KERNEL_OBJS`，所以还需要再使用一次`patsubst`函数，把`KERNEL_OBJS`中的`*.asm`文件替换为相应的`*.o`文件。

### Creating and Running Environment

#### 一点关于页表的东西
`lib/entry.asm`里有如下代码：
```
global uvpd
    uvpd equ (UVPT+(UVPT>>12)*4)
```

根据`include/mmlayout.h`，`[UVPT, UVPT+PTSIZE)`这段虚拟内存是当前 User Environment 的一共1024个页表（这些页表在虚拟地址上是连续的）。给定一个虚拟地址`va`，如何找到它的PTE? 如下图所示：

![](imgs/locate_pte.png)

所以从字面上看，`uvpd`就是`UVPT`这个虚拟地址对应的PTE的地址。注意到`kernel/env.c`的`env_setup_vm()`有这样一段代码：
```c
 e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_P | PTE_U;
```

User Environment 的页目录和页表是这样的：

<img src="imgs/user_pd_pt.png" width=800/>

`UVPT[UVPT>>12]`正好就是页目录的物理地址！真是绝了！

#### user environment 的编译链接
如6.828所述，我们把编译好的二进制文件用链接器的`-b binary`选项直接嵌入到 kernel 里面。下面解释一个用户程序(user environment)二进制（例如`obj/user/hello`）是如何生成的。

`user/`下面的每个C文件都包含一个`umain()`函数，相当于我们平时写C语言时的`main()`函数。我们平时写的用户程序会包含一个`main()`函数，但它不是程序的入口。程序启动后会进行必要的初始化，然后才会调用我们写的`main()`函数。`main()`函数返回后，调用者会执行一些清理工作，而后程序方能终止。在 JOS 里面，用户程序入口点位于`lib/entry.asm`的`_start`，把`ESP`设置好后会调用`lib/libmain.c`的`libmain()`:
```c
void libmain(int argc, char **argv) {
  // set thisenv to point at our Env structure in envs[].
  // LAB 3: Your code here.
  thisenv = 0;

  // save the name of the program so that panic() can use it
  if (argc > 0) {
    binaryname = argv[0];
  }

  // call user main routine
  umain(argc, argv);

  // exit gracefully
  // exit();
}
```

目前我们尚未实现系统调用，所以无法完成这个函数，`exit()`也要注释掉。在 Lab3-2 这一步我们只需要完成用户程序的编译链接，然后从 kernel 跳入，实现 ring0 到 ring3 的特权级转移。

编译链接一个用户程序二进制需要哪些源文件？根据上面的分析，至少需要：

1. `lib/entry.asm`——入口点
2. `lib/libmain.c`——用户程序的“main”函数`umain()`的调用者
3. `user/*.c`——用户程序的“main”函数`umain()`

此外，用户程序可能还会一些库函数，例如`printf()`。现在的`lib/`目录下有这几个源文件：
```
lib
├── Makefile
├── entry.asm
├── libmain.c
├── readline.c
├── string.c
└── vsprintf.c
```
我们把`readline.c`、`string.c`和`vsprintf.c`编译后的`*.o`文件单独打包成库文件`libjos.a`。详见`lib/Makefile`。不知道你是否注意到，这里没有`printf.c`，因为我把它挪到`kernel/`目录下了。原因是目前的`printf()`的实现方式是直接调用`kernel/console.c`定义的接口向显存写数据，这种操作是不能提供给 ring3 的用户程序的。另外，从编译链接的角度来讲也是不允许的——用户程序的编译链接居然需要内核源文件的参与？！等实现了系统调用后，我会像6.828一样在`lib/`下实现一个`printf.c`。

需要注意的是，`readline.c`、`string.c`和`vsprintf.c`也被用在了内核的编译链接中，参见`kernel/Makefile`。对于这几个源文件中出现的`printf()`、`putchar()`和`getchar()`几个函数，如果我们是在链接内核，那它们就会被链接到`kernel/`里的实现；如果是在链接用户程序，就会被链接到`lib/`里的实现——通过系统调用实现（因为 Lab3-2 尚未走到那一步，所以......）

现在可以编译链接我们的用户程序了，参见`user/Makefile`。用到的链接脚本`user.ld`来自6.824，把入口点（链接地址）定为虚拟地址`0x800020`。与内核不同，用户程序 ELF 各个 section 的两个加载地址`VMA`和`LMA`都是相等的虚拟地址，因为我们现在已经不会再直接使用物理地址了。

最后，把这些编译好的二进制嵌入内核，详见`kernel/Makefile`。可以从`obj/kernel/kernel.sym`里面看到这样的符号：`_binary_obj_user_hello_start`,`_binary_obj_user_hello_end`,`_binary_obj_user_hello_size`。使用`kernel/env.h`里的这个宏就可以将用户程序二进制提取出来：
```c
// Without this extra macro, we couldn't pass macros like TEST to
// ENV_CREATE because of the C pre-processor's argument prescan rule.
#define ENV_PASTE3(x, y, z) x##y##z

#define ENV_CREATE(x, type)                                \
  do {                                                     \
    extern uint8_t ENV_PASTE3(_binary_obj_, x, _start)[];  \
    env_create(ENV_PASTE3(_binary_obj_, x, _start), type); \
  } while (0)
```

#### Exercise 2

在`kernel/env.c`里实现这几个函数：

- [`env_init()`](#env_init)
- [`env_setup_vm()`](#env_setup_vm)
- [`region_alloc()`](#region_alloc)
- [`load_icode()`](#load_icode)
- [`env_create()`](#env_create)
- [`env_pop_tf()`](#env_pop_tf)
- [`env_run()`](#env_run)

##### `env_init()`
`i386_init()`调用它初始化`envs[]`，然后切换GDT，初始化段选择子（所有段选择子的`TI`位都为0，也就是说 JOS 不会使用LDT）。新的GDT内容如下：
```c
struct Segdesc gdt[] = {
    // 0x0 - unused (always faults -- for trapping NULL far pointers)
    SEG_NULL,

    // 0x8 - kernel code segment
    [GD_KT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),

    // 0x10 - kernel data segment
    [GD_KD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 0),

    // 0x18 - user code segment
    [GD_UT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),

    // 0x20 - user data segment
    [GD_UD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 3),

    // 0x28 - tss, initialized in trap_init_percpu()
    [GD_TSS0 >> 3] = SEG_NULL};

struct Pseudodesc gdt_pd = {sizeof(gdt) - 1, (unsigned long)gdt};
```

以下是描述符类型定义：
```c
// Application segment type bits
#define STA_X 0x8  // Executable segment
#define STA_E 0x4  // Expand down (non-executable segments)
#define STA_C 0x4  // Conforming code segment (executable only)
#define STA_W 0x2  // Writeable (non-executable segments)
#define STA_R 0x2  // Readable (executable segments)
#define STA_A 0x1  // Accessed
```

`type`在代码段和数据段有不同的含义：
|type位|说明|取值|
|-|-|-|
|**代码段**|||
|X: bit 3|代码段值为1|0: 数据段, 1: 代码段|
|C: bit 2|访问位|0: 非一致代码段, 1: 一致代码段|
|R: bit 1|是否可读|0: 只执行, 1: 可读|
|A: bit 0|是否被访问过|0: 未访问, 1: 已访问|
|**数据段**|||
|X: bit 3|代码段值为1|0: 数据段, 1: 代码段|
|E: bit 2|扩展方向|0: 向高地址扩展, 1: 向低地址扩展|
|W: bit 1|是否可写|0: 只读, 1: 可写|
|A: bit 0|是否被访问过|0: 未访问, 1: 已访问|

对于“一致/非一致代码段”的辨析此处从略。

##### `env_setup_vm()`

##### `region_alloc()`

##### `load_icode()`

##### `env_create()`

##### `env_pop_tf()`

##### `env_run()`
