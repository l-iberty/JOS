#include <include/assert.h>
#include <include/mmu.h>
#include <include/x86.h>
#include <kernel/console.h>
#include <kernel/env.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>
#include <kernel/syscall.h>
#include <kernel/trap.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = {{0}};
struct Pseudodesc idt_pd = {sizeof(idt) - 1, (uint32_t)idt};

static const char *trapname(int trapno) {
  static const char *const excnames[] = {"Divide error",
                                         "Debug",
                                         "Non-Maskable Interrupt",
                                         "Breakpoint",
                                         "Overflow",
                                         "BOUND Range Exceeded",
                                         "Invalid Opcode",
                                         "Device Not Available",
                                         "Double Fault",
                                         "Coprocessor Segment Overrun",
                                         "Invalid TSS",
                                         "Segment Not Present",
                                         "Stack Fault",
                                         "General Protection",
                                         "Page Fault",
                                         "(unknown trap)",
                                         "x87 FPU Floating-Point Error",
                                         "Alignment Check",
                                         "Machine-Check",
                                         "SIMD Floating-Point Exception"};

  if (trapno < ARRAY_SIZE(excnames)) return excnames[trapno];
  if (trapno == T_SYSCALL) return "System call";
  return "(unknown trap)";
}

void trap_init(void) {
  extern struct Segdesc gdt[];

  // LAB 3: Your code here.

  static const void *const excentries[] = {divide_error,
                                           debug,
                                           non_maskable_interrupt,
                                           breakpoint,
                                           overflow,
                                           bound_range_exceeded,
                                           invalid_opcode,
                                           device_not_avaliable,
                                           double_fault,
                                           coprocessor_segment_overrun,
                                           invalid_tss,
                                           segment_not_present,
                                           stack_fault,
                                           general_fault,
                                           page_fault,
                                           NULL,
                                           fpu_float_point_error,
                                           alignment_check,
                                           machine_check,
                                           simd_float_point_exception};

  int i;
  for (i = 0; i < ARRAY_SIZE(excentries); i++) {
    SETGATE(idt[i], 0, GD_KT, excentries[i], 0);
  }
  SETGATE(idt[T_BRKPT], 0, GD_KT, excentries[T_BRKPT], 3);

  SETGATE(idt[T_SYSCALL], 0, GD_KT, syscall_handler, 3);

  // Per-CPU setup
  trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void trap_init_percpu(void) {
  // The example code here sets up the Task State Segment (TSS) and
  // the TSS descriptor for CPU 0. But it is incorrect if we are
  // running on other CPUs because each CPU has its own kernel stack.
  // Fix the code so that it works for all CPUs.
  //
  // Hints:
  //   - The macro "thiscpu" always refers to the current CPU's
  //     struct CpuInfo;
  //   - The ID of the current CPU is given by cpunum() or
  //     thiscpu->cpu_id;
  //   - Use "thiscpu->cpu_ts" as the TSS for the current CPU,
  //     rather than the global "ts" variable;
  //   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
  //   - You mapped the per-CPU kernel stacks in mem_init_mp()
  //   - Initialize cpu_ts.ts_iomb to prevent unauthorized environments
  //     from doing IO (0 is not the correct value!)
  //
  // ltr sets a 'busy' flag in the TSS selector, so if you
  // accidentally load the same TSS on more than one CPU, you'll
  // get a triple fault.  If you set up an individual CPU's TSS
  // wrong, you may not get a fault until you try to return from
  // user space on that CPU.
  //
  // LAB 4: Your code here:

  // Setup a TSS so that we get the right stack
  // when we trap to the kernel.
  thiscpu->cpu_ts.ts_esp0 = KSTACKTOP - cpunum() * (KSTKSIZE + KSTKGAP);
  thiscpu->cpu_ts.ts_ss0 = GD_KD;
  thiscpu->cpu_ts.ts_iomb = sizeof(struct Taskstate);

  // Initialize the TSS slot of the gdt.
  gdt[(GD_TSS0 >> 3) + cpunum()] = SEG16(STS_T32A, (uint32_t)(&thiscpu->cpu_ts), sizeof(struct Taskstate) - 1, 0);
  gdt[(GD_TSS0 >> 3) + cpunum()].sd_s = 0;

  // Load the TSS selector
  ltr(GD_TSS0 + (cpunum() << 3));

  // Setup a TSS so that we get the right stack
  // when we trap to the kernel.
  // ts.ts_esp0 = KSTACKTOP;
  // ts.ts_ss0 = GD_KD;
  // ts.ts_iomb = sizeof(struct Taskstate);

  // Initialize the TSS slot of the gdt.
  // gdt[GD_TSS0 >> 3] =
  //     SEG16(STS_T32A, (uint32_t)(&ts), sizeof(struct Taskstate) - 1, 0);
  // gdt[GD_TSS0 >> 3].sd_s = 0;

  // Load the TSS selector (like other segment selectors, the
  // bottom three bits are special; we leave them 0)
  // ltr(GD_TSS0);

  // Load the IDT
  lidt(&idt_pd);
}

void print_trapframe(struct Trapframe *tf) {
  printf("TRAP frame at %p\n", tf);
  print_regs(&tf->tf_regs);
  printf("  es   ----0x%04x\n", tf->tf_es);
  printf("  ds   ----0x%04x\n", tf->tf_ds);
  printf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
  // If this trap was a page fault that just happened
  // (so %cr2 is meaningful), print the faulting linear address.
  if (tf == last_tf && tf->tf_trapno == T_PGFLT) {
    printf("  cr2  0x%08x\n", rcr2());
  }
  printf("  err  0x%08x", tf->tf_err);
  // For page faults, print decoded fault error code:
  // U/K=fault occurred in user/kernel mode
  // W/R=a write/read caused the fault
  // PR=a protection violation caused the fault (NP=page not present).
  if (tf->tf_trapno == T_PGFLT) {
    printf(" [%s, %s, %s]\n", tf->tf_err & 4 ? "user" : "kernel", tf->tf_err & 2 ? "write" : "read",
           tf->tf_err & 1 ? "protection" : "not-present");
  } else {
    printf("\n");
  }
  printf("  eip  0x%08x\n", tf->tf_eip);
  printf("  cs   ----0x%04x\n", tf->tf_cs);
  printf("  flag 0x%08x\n", tf->tf_eflags);
  if ((tf->tf_cs & 3) != 0) {
    printf("  esp  0x%08x\n", tf->tf_esp);
    printf("  ss   0x%08x\n", tf->tf_ss);
  }
}

void print_regs(struct PushRegs *regs) {
  printf("  edi  0x%08x\n", regs->reg_edi);
  printf("  esi  0x%08x\n", regs->reg_esi);
  printf("  ebp  0x%08x\n", regs->reg_ebp);
  printf("  oesp 0x%08x\n", regs->reg_oesp);
  printf("  ebx  0x%08x\n", regs->reg_ebx);
  printf("  edx  0x%08x\n", regs->reg_edx);
  printf("  ecx  0x%08x\n", regs->reg_ecx);
  printf("  eax  0x%08x\n", regs->reg_eax);
}

static void trap_dispatch(struct Trapframe *tf) {
  // Handle processor exceptions.
  // LAB 3: Your code here.

  int32_t r;

  switch (tf->tf_trapno) {
    case T_DEBUG:
      // 当 TF=1 时, CPU执行完一条指令后产生单步中断, 进入中断处理程序后 TF
      // 自动置0
      assert(!(read_eflags() & FL_TF));
      printf("#DB  eip  0x%08x\n", tf->tf_eip);
      monitor(tf);
      return;
    case T_BRKPT:
      printf("#BP  eip  0x%08x\n", tf->tf_eip);
      monitor(tf);
      return;
    case T_PGFLT:
      page_fault_handler(tf);
      return;
    case T_SYSCALL:
      r = syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx,
                  tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
      tf->tf_regs.reg_eax = r;
      return;
  }

  // Unexpected trap: The user process or the kernel has a bug.
  print_trapframe(tf);
  if (tf->tf_cs == GD_KT) {
    panic("unhandled trap in kernel");
  } else {
    env_destroy(curenv);
    return;
  }
}

void trap(struct Trapframe *tf) {
  // The environment may have set DF and some versions
  // of GCC rely on DF being clear
  asm volatile("cld" ::: "cc");

  // Check that interrupts are disabled.  If this assertion
  // fails, DO NOT be tempted to fix it by inserting a "cli" in
  // the interrupt path.
  assert(!(read_eflags() & FL_IF));

  printf("Incoming TRAP frame at %p\n", tf);

  if ((tf->tf_cs & 3) == 3) {
    // Trapped from user mode.
    assert(curenv);

    // Copy trap frame (which is currently on the stack)
    // into 'curenv->env_tf', so that running the environment
    // will restart at the trap point.
    curenv->env_tf = *tf;
    // The trapframe on the stack should be ignored from here on.
    tf = &curenv->env_tf;
  }

  // Record that tf is the last real trapframe so
  // print_trapframe can print some additional information.
  last_tf = tf;

  // Dispatch based on what type of trap occurred
  trap_dispatch(tf);

  // Return to the current environment, which should be running.
  assert(curenv && curenv->env_status == ENV_RUNNING);
  env_run(curenv);
}

void page_fault_handler(struct Trapframe *tf) {
  uint32_t fault_va;

  // Read processor's CR2 register to find the faulting address
  fault_va = rcr2();

  // Handle kernel-mode page faults.

  // LAB 3: Your code here.
  if (tf->tf_cs == GD_KT) {
    panic("kernel-mode page faults! fault_va: %08x", fault_va);
  }

  // We've already handled kernel-mode exceptions, so if we get here,
  // the page fault happened in user mode.

  // Destroy the environment that caused the fault.
  printf("[%08x] user fault va %08x ip %08x\n", curenv->env_id, fault_va, tf->tf_eip);
  print_trapframe(tf);
  env_destroy(curenv);
}
