# Lab4-5

## Part C: Preemptive Multitasking and Inter-Process communication (IPC)

### Implementing IPC

Implement `sys_ipc_recv` and `sys_ipc_try_send` in `kern/syscall.c`.

```c
static int sys_ipc_recv(void *dstva) {
  // LAB 4: Your code here.

  if ((uintptr_t)dstva < UTOP) {
    if (!PAGE_ALIGNED(dstva)) {
      return -E_INVAL;
    }
  }

  curenv->env_ipc_recving = true;
  curenv->env_ipc_from = 0;
  curenv->env_ipc_dstva = dstva;
  curenv->env_status = ENV_NOT_RUNNABLE;

  sched_yield();
  return 0;
}

static int sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm) {
  // LAB 4: Your code here.

  struct Env *e;
  int r;

  if ((r = envid2env(envid, &e, false)) < 0) {
    return r;
  }

  if (!e->env_ipc_recving) {
    return -E_IPC_NOT_RECV;
  }

  if ((uintptr_t)srcva < UTOP) {
    if (!PAGE_ALIGNED(srcva)) {
      return -E_INVAL;
    }
    if (e->env_ipc_dstva != 0) {
      if ((r = sys_page_map_ex(0, srcva, envid, e->env_ipc_dstva, perm, false)) < 0) {
        return r;
      }
    }
  }

  e->env_ipc_recving = false;
  e->env_ipc_from = curenv->env_id;
  e->env_ipc_value = value;
  e->env_ipc_perm = (uintptr_t)srcva < UTOP ? perm : 0;
  e->env_tf.tf_regs.reg_eax = 0;
  e->env_status = ENV_RUNNABLE;

  return 0;
}
```

Then implement the `ipc_recv` and `ipc_send` functions in `lib/ipc.c`.

```c
void ipc_send(envid_t to_env, uint32_t val, void *pg, int perm) {
  // LAB 4: Your code here.

  int r;
  void *va = pg ? pg : (void *)UTOP;

  while ((r = sys_ipc_try_send(to_env, val, va, perm)) < 0) {
    if (r != -E_IPC_NOT_RECV) {
      panic("sys_ipc_try_send");
    }
    sys_yield();
  }
}

int32_t ipc_recv(envid_t *from_env_store, void *pg, int *perm_store) {
  // LAB 4: Your code here.

  int r;
  void *va = pg ? pg : (void *)UTOP;

  if ((r = sys_ipc_recv(va)) < 0) {
    if (from_env_store) {
      *from_env_store = 0;
    }
    if (perm_store) {
      *perm_store = 0;
    }
    return r;
  }

  if (from_env_store) {
    *from_env_store = thisenv->env_ipc_from;
  }
  if (perm_store) {
    *perm_store = thisenv->env_ipc_perm;
  }

  return thisenv->env_ipc_value;
}
```

#### 说明1

系统调用`sys_ipc_recv`会把 receiver 进程标记为`ENV_NOT_RUNNABLE`，最后调用`sched_yield`，从而让 receiver 放弃CPU。当 sender 进程想要给 receiver 发消息时，它会把 receiver 标记为`ENV_RUNNABLE`。之后发生时钟中断时 receiver 就有可能被调度到。那么 receiver 下次被调度时从哪个地方继续执行？receiver 发起系统调用的时候它的上下文被保存在自己的`Trapframe`里面，查看其中内容即可。

首先在`trap_dispatch`的`T_SYSCALL`分支打一个条件断点：

```c
static void trap_dispatch(struct Trapframe *tf) {
  // Handle processor exceptions.
  // LAB 3: Your code here.

  int32_t r;

  switch (tf->tf_trapno) {
    ......
    case T_SYSCALL:
      r = syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx,
                  tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
      tf->tf_regs.reg_eax = r;
      return;
  }
  ......
```

GDB命令：

```
symbol-file obj/kernel/kernel
b kernel/trap.c:252
condition 1 tf->tf_regs.regs_eax==12
```

注意`SYS_ipc_recv=12`

然后查看`tf_eip`:

```
gdb-peda$ p tf->tf_eip
$2 = 0x80069c
```

打开用户进程的反汇编，以`obj/user/primes.objdump`为例，找到`0x80069c`：

```
00800680 <syscall>:
  800680:	55                   	push   %ebp
  800681:	89 e5                	mov    %esp,%ebp
  800683:	52                   	push   %edx
  800684:	51                   	push   %ecx
  800685:	57                   	push   %edi
  800686:	56                   	push   %esi
  800687:	53                   	push   %ebx
  800688:	8b 45 08             	mov    0x8(%ebp),%eax
  80068b:	8b 55 0c             	mov    0xc(%ebp),%edx
  80068e:	8b 4d 10             	mov    0x10(%ebp),%ecx
  800691:	8b 5d 14             	mov    0x14(%ebp),%ebx
  800694:	8b 7d 18             	mov    0x18(%ebp),%edi
  800697:	8b 75 1c             	mov    0x1c(%ebp),%esi
  80069a:	cd 30                	int    $0x30
 [80069c]:	5b                   	pop    %ebx
  80069d:	5e                   	pop    %esi
  80069e:	5f                   	pop    %edi
  80069f:	59                   	pop    %ecx
  8006a0:	5a                   	pop    %edx
  8006a1:	5d                   	pop    %ebp
  8006a2:	c3                   	ret
```

位于用户层的系统调用接口`syscall` in `lib/syscall.c`，`0x80069c`指向`int $0x30`后的第一条指令。

`syscall`的返回值保存在`eax`。`env_run`调用`env_pop_tf`从`Trapframe`恢复用户进程的上下文。所以可以通过设置`Trapframe`的`tf_regs.regs_eax`字段来让用户进程的系统调用返回我们期望的值。实现`fork`的时候通过把子进程的`regs_eax`设置成0，使得子进程的`fork`返回0。对于`sys_ipc_recv`也一样，通过把 receiver 的`regs_eax`设置成0来让其从系统调用中返回0，表示成功。

#### 说明2

`sys_ipc_try_send`需要在 sender 和 receiver 之间建立页面映射，但不能直接调用原来的`sys_page_map`，因为 IPC 允许任意两个进程发消息，不局限于父子进程之间。所以我写了一个`sys_page_map_ex`，携带一个`checkperm`参数。原来的`sys_page_map`调用`sys_page_map_ex`时`checkperm=true`。`sys_ipc_try_send`调用`sys_page_map_ex`时`checkperm=false`。

### 一些BUGS

1. 在`lib/_syscall.asm`里实现的`syscall`没有正确地保存和恢复现场, 修改如下:

```
[SECTION .text]
global syscall
; int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
syscall:
    push  ebp
    mov   ebp, esp
    push  edx
    push  ecx
    push  edi
    push  esi
    push  ebx

    mov   eax, [ebp+8] ; num
    mov   edx, [ebp+12] ; a1
    mov   ecx, [ebp+16] ; a2
    mov   ebx, [ebp+20] ; a3
    mov   edi, [ebp+24] ; a4
    mov   esi, [ebp+28] ; a5
    int   0x30

    pop   ebx
    pop   esi
    pop   edi
    pop   ecx
    pop   edx
    pop   ebp
    ret
```

2. `sched_yield()`的问题直到`user/primes`测试才暴露出来。因为一些进程无法被调度到，导致 IPC 被无限阻塞。修改如下:

```c
// Choose a user environment to run and run it.
void sched_yield() {
  struct Env *idle;

  // Implement simple round-robin scheduling.
  //
  // Search through 'envs' for an ENV_RUNNABLE environment in
  // circular fashion starting just after the env this CPU was
  // last running.  Switch to the first such environment found.
  //
  // If no envs are runnable, but the environment previously
  // running on this CPU is still ENV_RUNNING, it's okay to
  // choose that environment.
  //
  // Never choose an environment that's currently running on
  // another CPU (env_status == ENV_RUNNING). If there are
  // no runnable environments, simply drop through to the code
  // below to halt the cpu.

  // LAB 4: Your code here.
  int i, j, envx;
  envx = curenv ? ENVX(curenv->env_id) : 0;

  for (i = 0; i < NENV; i++) {
    j = (envx + i) % NENV;
    if (envs[j].env_status == ENV_RUNNABLE) {
      env_run(&envs[j]);
    }
  }

  if (curenv && curenv->env_status == ENV_RUNNING) {
    env_run(curenv);
  }

  // sched_halt never returns
  sched_halt();
}
```

### `curenv` vs `thisenv`

- `curenv`是内核代码里的变量，定义在`kernel/env.h`: `#define curenv (thiscpu->cpu_env)`。表示当前 CPU 上运行的用户进程。根据进入临界区的 CPU 的不同，`curenv`将指向不同的`Env`结构，因为一个用户进程不会同时运行在多个 CPU 上。
- `thisenv`是用户代码里的变量，定义在`lib/libmain.c`: `const volatile struct Env *thisenv;`。在`libmain`函数里完成初始化: `thisenv = &envs[ENVX(sys_getenvid())];`。fork 产生子进程后，在子进程的地址空间内需要修正`thisenv`指向子进程自己。所以在不同用户进程的代码里`thisenv`也是不同的——每个进程的地址空间里都有一个名为`thisenv`的变量，指向各自进程的`Env`结构。
- 在用户进程里调用`sys_getenvid`，内核返回的是`curenv->end_id`，这与直接从`thisenv->env_id`得到的值是一致的。`thisenv`相当于给用户进程提供了一个访问自身`Env`结构的途径。如果一个用户进程暂时没有获得 CPU，虽然在它自己的地址空间内`thisenv`依然指向它自己，但`curenv`就不再指向这个进程了。只有当一个用户进程获得 CPU 时才满足`thisenv == curenv`。