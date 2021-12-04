# Lab4-3

## Part A: Multiprocessor Support and Cooperative Multitasking

### System Calls for Environment Creation

- `sys_exofork`
- `sys_env_set_status`
- `sys_page_alloc`
- `sys_page_map`
- `sys_page_unmap`

`user/dumbfork`从 6.828 拷贝过来即可，我在其中加了一些注释。最后是运行结果：

<img src="imgs/demo.png" width=700/>

### User-level page fault handling

`struct Env`有多了一个新成员：

```c
struct Env {
  ......
  // Exception handling
  void *env_pgfault_upcall;  // Page fault upcall entry point
};
```

#### Setting the Page Fault Handler
It's easy to implement `sys_env_pgfault_upcall`:

```c
static int sys_env_set_pgfault_upcall(envid_t envid, void *func) {
  // LAB 4: Your code here.

  struct Env *e;
  int r;

  if ((r = envid2env(envid, &e, true)) < 0) {
    return r;
  }

  e->env_pgfault_upcall = func;
  return 0;
}
```

#### Invoking the User Page Fault Handler
修改位于`kernel/trap.c`的`page_fault_handler()`，实现对 user-mode page fault 的处理：

```c
void page_fault_handler(struct Trapframe *tf) {
  ......

  // LAB 4: Your code here.

  struct UTrapframe *utf;
  int r;

  if (curenv->env_pgfault_upcall == NULL) {
    goto bad;
  }

  user_mem_assert(curenv, (const void *)(UXSTACKTOP - PGSIZE), PGSIZE, PTE_P | PTE_U | PTE_W);

  if (tf->tf_esp >= UXSTACKTOP - PGSIZE && tf->tf_esp < UXSTACKTOP) {
    utf = (struct UTrapframe *)(tf->tf_esp - 4 - sizeof(struct UTrapframe));
  } else {
    utf = (struct UTrapframe *)(UXSTACKTOP - sizeof(struct UTrapframe));
  }

  utf->utf_fault_va = fault_va;
  utf->utf_err = T_PGFLT;
  utf->utf_regs = tf->tf_regs;
  utf->utf_eip = tf->tf_eip;
  utf->utf_eflags = tf->tf_eflags;
  utf->utf_esp = tf->tf_esp;

  curenv->env_tf.tf_eip = (uintptr_t)(curenv->env_pgfault_upcall);
  curenv->env_tf.tf_esp = (uintptr_t)utf;
  env_run(curenv);

bad:
  // Destroy the environment that caused the fault.
  printf("[%08x] user fault va %08x ip %08x\n", curenv->env_id, fault_va, tf->tf_eip);
  print_trapframe(tf);
  env_destroy(curenv);
}
```

按照注释提示，首先进行一些检查，然后开始在 Exception Stack 上面布置一个`struct UTrapframe`，结构如下：

```c
struct UTrapframe {
  /* information about the fault */
  uint32_t utf_fault_va; /* va for T_PGFLT, 0 otherwise */
  uint32_t utf_err;
  /* trap-time return state */
  struct PushRegs utf_regs;
  uintptr_t utf_eip;
  uint32_t utf_eflags;
  /* the trap-time stack to return to */
  uintptr_t utf_esp;
} __attribute__((packed));
```

如果 user environment 在正常执行过程中发生 #PF，那么`tf->tf_esp`位于 Normal Stack 范围内，即`[UXSTACKTOP-PGSIZE, UXSTACKTOP-1]`。此时我们需要在`UXSTACKTOP`之下布置一个`struct UTrapframe`：

```
[高地址]
                    <-- UXSTACKTOP
trap-time esp
trap-time eflags
trap-time eip
trap-time eax       start of struct PushRegs
trap-time ecx
trap-time edx
trap-time ebx
trap-time esp
trap-time ebp
trap-time esi
trap-time edi       end of struct PushRegs
tf_err (error code)
fault_va            <-- %esp when handler is run

[低地址]
```

所以要这样设置`utf`指针：

```c
utf = (struct UTrapframe *)(UXSTACKTOP - sizeof(struct UTrapframe));
```

才能正确地填充数据。

为什么一定要把`UTrapframe`刚好布置在`UXSTACKTOP`之下？因为栈是从高地址向低地址方向生长的。这么做相当于向 Exception Stack 里面"push"一个`UTrapframe`，而栈指针的初值为`UXSTACKTOP`。

完成上述工作后需要转移到 user environment 注册的*page fault handler entrypoint*，此时会发生 ring0 到 ring3 的特权级转移，借助`env_run()`实现：

```c
  curenv->env_tf.tf_eip = (uintptr_t)(curenv->env_pgfault_upcall);
  curenv->env_tf.tf_esp = (uintptr_t)utf;
  env_run(curenv);
```

**注意：** `env_tf.tf_eip`和`env_tf.tf_esp`被修改了之后如何返回 user environment 触发 #PF 的位置重新执行？事实上`trap-time eip、eflags、esp`已经被保存到之前在 Exception Stack 上布置的`UTrapframe`里面了，后面会从 Exception Stack 上返回。

借助`env_run()`会跳转到`_pgfault_upcall`，位于`lib/pfentry.asm`：

```
_pgfault_upcall:
    ; call the C page fault handler
    push  esp                     ; function argument: pointer to UTrapframe
    mov   eax, [_pgfault_handler]
    call  eax
    add   esp, 4                  ; pop function argument

    ; Now the C page fault handler has returned and you must return
    ; to the trap time state.
    ; Push trap-time %eip onto the trap-time stack.
    ;
    ; LAB 4: Your code here.
    mov  eax, [esp + 40]    ; %eax <- trap-time %eip
    mov  ebx, [esp + 48]    ; %ebx <- trap-time %esp
    mov  [ebx - 4], eax     ; "push" trap-time %eip onto the trap-time stack

    ; Restore the trap-time registers.  After you do this, you
    ; can no longer modify any general-purpose registers.
    ; LAB 4: Your code here.
    add  esp, 8             ; skip utf_fault_va and utf_err
    popa                    ; restore general-purpose registers from utf_regs

    ; Restore eflags from the stack.  After you do this, you can
    ; no longer use arithmetic operations or anything else that
    ; modifies eflags.
    ; LAB 4: Your code here.
    add  esp, 4             ; skip utf_eip
    popf                    ; restore eflags from the stack

    ; Switch back to the adjusted trap-time stack.
    ; LAB 4: Your code here.
    pop  esp                ; pop utf_esp into %esp

    ; Return to re-execute the instruction that faulted.
    ; LAB 4: Your code here.
    sub  esp, 4
    ret
```

刚进入`_pgfault_upcall`时的栈帧：

```
[高地址]
/* +48 */ trap-time esp
/* +44 */ trap-time eflags
/* +40 */ trap-time eip
/* +36 */ utf_regs.reg_eax
          ...
/* +12 */ utf_regs.reg_esi
/* +8  */ utf_regs.reg_edi
/* +4  */ utf_err (error code)
/* +0  */ utf_fault_va            <-- %esp
[低地址]
```

这就是之前布置好的`UTrapframe`。`push esp`压入栈的就是这个`UTrapframe`的指针(栈向低地址方向生长，`push`不会破环这个`UTrapframe`)，然后`call eax`跳转到 C 函数`_pgfault_handler`(`lib/pgfault.c`)。C 函数返回后`add esp, 4`将`esp`退回原先的位置，之后便可根据偏移量访问`UTrapframe`里的各个成员。

**1. Push trap-time `%eip` onto the trap-time stack.**

目前处在 Exception Stack，而 trap-time stack 可能是 Normal Stack 或 Exception Stack——总之不能直接用`push`指令进行这里的"push"。必须从把 trap-time esp 先取到`ebx`里面，再把 trap-time eip 写到`[ebx-4]`——为什么要`-4`？因为`push xxx`可以分解为：

```
 (1) sub esp, 4
 (2) mov [esp], xxx
```

所以要模拟`push`指令的动作就必须`-4`，否则直接`mov [ebx], eax`会破坏掉 trap-time stack 上面的数据。

**2. Restore the trap-time registers.**

`add esp, 8`跳过`utf_fault_va`和`utf_err`，方能`popa`.

**3. Restore eflags from the stack.**

上一条指令`popa`之后`esp`指向 trap-time eip，所以需要`add esp, 4`跳过之，使得`esp`指向 trap-time eflags，然后`popf`将其弹入`eflags`.

**4. Switch back to the adjusted trap-time stack.**

当`esp`指向 trap-time esp 时执行`pop esp`即可切换到 trap-time stack.

**5. Return to re-execute the instruction that faulted.**

这一步需要把 trap-time eip 打到`eip`里面。当前的`esp`即为 trap-time esp，而在`esp-4`这个地址上的 32-bit word 就是第一步里保存进去的 trap-time eip. 首先`sub esp,4`，然后`ret`把 trap-time eip 打到`eip`里就完成可转移。`ret`相当于`pop eip`，所以`esp`也被**同时**重新调整为 trap-time esp 了。——这就是**Exercise 10**要求我们做的：*The hard part is simultaneously switching stacks and re-loading the EIP.*


现在再来分析这个问题：

*If the user environment is already running on the user exception stack when an exception occurs, then the page fault handler itself has faulted. In this case, you should start the new stack frame just under the current `tf->tf_esp` rather than at `UXSTACKTOP`. You should first push an empty 32-bit word, then a `struct UTrapframe`.*

*To test whether `tf->tf_esp` is already on the user exception stack, check whether it is in the range between `UXSTACKTOP-PGSIZE` and `UXSTACKTOP-1`, inclusive.*

此时我们不能把`UTrapframe`布置在`UXSTACKTOP`之下——因为那里已经有一个`UTrapframe`了。我们在 Exception Stack 范围内再布置一个`UTrapframe`，具体位置就在`tf->tf_esp`之下，但不能紧贴着`tf->tf_esp`，因为我们需要保留一个 32-bit word —— 后面会把 trap-time eip 保存到`tf->tf_esp-4`这个地址上。所以在`page_fault_handler()`里这么写：

```c
if (tf->tf_esp >= UXSTACKTOP - PGSIZE && tf->tf_esp < UXSTACKTOP) {
  utf = (struct UTrapframe *)(tf->tf_esp - 4 - sizeof(struct UTrapframe));
}
```

最后的栈帧：

```
[高地址]
  ┌─────────────────────┐
  │                     │  <-- tf->tf_esp
  ├─────────────────────┤
  │    32-bit word      │
  ├─────────────────────┤
  │                     │
  │                     │
  │     UTrapframe      │
  │                     │
  │                     │
  └─────────────────────┘
[低地址]
```

#### 测试
1. `user/faultalloc`

<img src="imgs/faultalloc.png" width=700/>

2. `user/faultallocbad`

<img src="imgs/faultallocbad.png" width=700/>

输出结果和 6.828 预期的一致。来自 6.828 对`user/faultallocbad`的提问：*alloc pages to fix faults doesn't work because we `sys_cputs` instead of `cprintf` (exercise: why?)*

`printf("%s\n", (char *)fault_va)`打印`fault_va`地址上的字符串时，存在一个从`fault_va`拷贝字符串到 buffer 的过程——此时会触发 #PF，然后用户程序的 page fault handler 就会被调用，完成相应的处理。最后才会调用`sys_puts`打印 buffer 里的内容。

`sys_puts((char *)fault_va, 4)`进入内核后是这样做的：

```c
static void sys_puts(const char *s, size_t len) {
  // Check that the user has permission to read memory [s, s+len).
  // Destroy the environment if not.

  // LAB 3: Your code here.
  user_mem_assert(curenv, s, len, 0);

  // Print the string supplied by the user.
  printf("%.*s", len, s);
}
```

直接对`fault_va`调用`user_mem_assert`进行检查，此时 #PF 尚未被触发，用户程序的 page fault handler 也未被调用，所以检查就会失败。
