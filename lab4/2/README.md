# Lab4-2

## Part A: Multiprocessor Support and Cooperative Multitasking

### Locking

spinlock 的实现要点：

- `xchg`指令有两个目的：(1) 把“读出旧值、写入新值”两个操作合二为一，变成一个原子操作, (2) `xchg`指令会向 CPU 宣告一个 LOCK# 信号，这样就能确保在多处理器系统或多线程竞争的环境下互斥地使用这个内存地址。当`xchg`指令执行完毕，LOCK# 就会消失。LOCK# 会引起处理器缓存回写到内存，它相当于一个内存栅栏，主要提供3个功能：

	- 确保指令重排序时不会把其后面的指令排到内存栅栏之前的位置，也不会把前面的指令排到内存栅栏的后面；即在执行到内存栅栏这条指令时，在它前面的操作已经全部完成；
	- 强制将对缓存的修改操作立即写入主存 ，利用缓存一致性机制阻止同时修改由两个以上 CPU 缓存的内存区域数据；
	- 如果是写操作，它会导致其他 CPU 中对应的 cache line 无效。

- 为什么`while`循环不能空转，而是要插入一条`pause`指令？(与流水线、分支预测有关，主要目的是优化 CPU 性能，我也不太明白)

参考：

[https://stackoverflow.com/questions/36731166/spinlock-with-xchg-unlocking](https://stackoverflow.com/questions/36731166/spinlock-with-xchg-unlocking)

[https://stackoverflow.com/questions/4725676/how-does-x86-pause-instruction-work-in-spinlock-and-can-it-be-used-in-other-sc](https://stackoverflow.com/questions/4725676/how-does-x86-pause-instruction-work-in-spinlock-and-can-it-be-used-in-other-sc)

[https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86](https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86)