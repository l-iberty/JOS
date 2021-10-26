# Lab3-2

## Part A: User Environments and Exception Handling

### Handling Interrupts and Exceptions

TSS的作用：ring3 向 ring0-2 转移时需要切换栈，所以 TSS 里定义了 ring0-2 的`ss`和`esp`