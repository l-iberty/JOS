#ifndef JOS_KERN_SPINLOCK_H
#define JOS_KERN_SPINLOCK_H

#include <include/types.h>

// mutual exclusion lock
struct spinlock {
  unsigned locked;  // Is the lock held?

  char *name;           // Name of the lock.
  struct CpuInfo *cpu;  // The CPU holding the lock.
};

void __spin_initlock(struct spinlock *lk, char *name);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);

#define spin_initlock(lock) __spin_initlock(lock, #lock)

extern struct spinlock kernel_lock;

static inline void lock_kernel() { spin_lock(&kernel_lock); }

static inline void unlock_kernel() {
  spin_unlock(&kernel_lock);

  // Normally we wouldn't need to do this, but QEMU only runs
  // one CPU at a time and has a long time-slice.  Without the
  // pause, this CPU is likely to reacquire the lock before
  // another CPU has even been given a chance to acquire it.
  asm volatile("pause");
}

#endif /* JOS_KERN_SPINLOCK_H */
