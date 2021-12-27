#include <include/assert.h>
#include <include/string.h>
#include <include/types.h>
#include <include/x86.h>
#include <kernel/cpu.h>
#include <kernel/spinlock.h>

// The big kernel lock
struct spinlock kernel_lock = {.name = "kernel_lock"};

// Check whether this CPU is holding the lock.
static bool holding(struct spinlock *lk) { return lk->locked && lk->cpu == thiscpu; }

void __spin_initlock(struct spinlock *lk, char *name) {
  lk->locked = 0;
  lk->name = name;
  lk->cpu = NULL;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void spin_lock(struct spinlock *lk) {
  if (holding(lk)) {
    panic("CPU %d cannot acquire %s: already holding", cpunum(), lk->name);
  }

  // The xchg is atomic.
  // It also serializes, so that reads after acquire are not
  // reordered before it.
  while (xchg(&lk->locked, 1) != 0) {
    asm volatile("pause");
  }

  lk->cpu = thiscpu;
}

// Release the lock.
void spin_unlock(struct spinlock *lk) {
  if (!holding(lk)) {
    panic("CPU %d cannot release %s: not holding", cpunum(), lk->name);
  }

  lk->cpu = NULL;

  // The xchg instruction is atomic. (i.e. use the "lock" prefix) with
  // respect to any other instruction which references the same memory.
  // x86 CPUs will not reorder loads/stores across locked instructions.
  // (vol 3, 8.2.2). Because xchg() is implemented using assembly, gcc
  // will not reorder C statements acrosss the xchg.
  xchg(&lk->locked, 0);
}