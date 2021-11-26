# Lab4-2

## Part A: Multiprocessor Support and Cooperative Multitasking

### Locking

spinlock 的实现要点：

- `xchg`指令有两个目的：(1) 把“读出旧值、写入新值”两个操作合二为一，变成一个原子操作, (2) `xchg`会向 CPU 宣告一个 LOCK# 信号锁住总线(486以后大概是锁 cache line + MESI 协议了)，这样就能确保在多处理器系统或多线程竞争的环境下互斥地使用这个内存地址。当`xchg`执行完毕，LOCK# 就会消失。LOCK# 会引起处理器缓存回写到内存，它相当于一个内存栅栏，主要提供3个功能：

	- 确保指令重排序时不会把其后面的指令排到内存栅栏之前的位置，也不会把前面的指令排到内存栅栏的后面；即在执行到内存栅栏这条指令时，在它前面的操作已经全部完成；
	- 强制将对缓存的修改操作立即写入主存 ，利用缓存一致性机制阻止同时修改由两个以上 CPU 缓存的内存区域数据；
	- 如果是写操作，它会导致其他 CPU 中对应的 cache line 无效。

`xchg`实现了内存一致性模型中的**顺序一致性**。顺序一致性指的是，多个处理器在并行执行一组内存读写操作时，所有处理器要维护一个单一的操作次序，即所有处理器看到的操作次序要一致。

如果在实现 spinlock 时对标记变量的读写采用`mov`指令，首先“读出旧值、写入新值”的原子性无法保证，更重要的是，可能因缓存不一致而导致如下问题：

1. CPU1 执行`mov [locked], 1`获得 spinlock 后进入临界区访问临界资源 (这里对变量`locked`的写操作肯定是要先更改cache, 至于是否立即写入主存则取决于变量所在的虚页是采用 write-through 还是 write-back 机制, 对应页表中的属性位)；
2. CPU2 在 CPU1 进入临界区之后执行`mov eax, [locked]`(假设将变量的值读入`eax`)时, CPU2 对应`locked`变量所在内存的 cache line 里的数据还是0, 并且`mov`是从 cache 读取数据, 那么 CPU2 就会认为当前没有其他核心进入临界区, 从而也能获得 spinlock 进入临界区.

如果 CPU1 采用`xchg`读写标记变量，就能通过 LOCK# 信号消除上述因缓存不一致而导致的问题。另外，在指令乱序执行时 LOCK# 起到了内存栅栏的作用，保证了`xchg`前面的指令不会被重排到`xchg`后面、`xchg`后面的指令不会被重排到`xchg`前面，这一点是至关重要的，因为`xchg`后面的指令显然是需要 CPU 获得了 spinlock 后才能执行的，决不能让它们在`xchg`之前（也就是获得 spinlock 之前）执行。

- 为什么`while`循环不能空转，而是要插入一条`pause`指令？(与流水线、分支预测有关，主要目的是优化 CPU 性能，我也不太明白)

参考：

[https://stackoverflow.com/questions/36731166/spinlock-with-xchg-unlocking](https://stackoverflow.com/questions/36731166/spinlock-with-xchg-unlocking)

[https://stackoverflow.com/questions/4725676/how-does-x86-pause-instruction-work-in-spinlock-and-can-it-be-used-in-other-sc](https://stackoverflow.com/questions/4725676/how-does-x86-pause-instruction-work-in-spinlock-and-can-it-be-used-in-other-sc)

[https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86](https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86)

最后是C函数声明：
```c
uint32_t xchg(volatile uint32_t *addr, uint32_t newval);
```
`volatile`关键字是被设计用来修饰被不同线程访问和修改的变量。`volatile`提醒编译器它后面所定义的变量随时都有可能改变，因此编译后的程序每次需要存储或读取这个变量的时候，告诉编译器对该变量不做优化，直接从变量内存地址中读取数据，从而可以提供对特殊地址的稳定访问。如果没有`volatile`关键字，则编译器可能优化读取和存储，可能暂时使用寄存器中的值，如果这个变量由别的程序更新了的话，将出现不一致的现象。

事实上，我们对`addr`地址处的变量存取使用的是`xchg`指令，而且这个C函数是直接用汇编实现的，甚至还不是 GNU 内联汇编！所以即使没有`volatile`关键字编译器也做不了那样的优化，这样做只是为了和`xchg`指令组成双保险。