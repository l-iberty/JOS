#include <include/assert.h>
#include <include/x86.h>
#include <kernel/env.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>

void sched_halt() __attribute__((noreturn));

// Choose a user environment to run and run it.
void sched_yield() {
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
  int i;
  for (i = 0; i < NENV; i++) {
    if (envs[i].env_status == ENV_RUNNABLE) {
      env_run(&envs[i]);
      panic("env_run should not return");
    }
  }
  if (curenv && curenv->env_status == ENV_RUNNING) {
    env_run(curenv);
    panic("env_run should not return");
  }

  // sched_halt never returns
  sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void sched_halt() {
  printf("No runnable enviroments in the system!\n");
  while (1) {
    monitor(NULL);
  }
}