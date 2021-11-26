#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H

// This function does not return.
void sched_yield() __attribute__((noreturn));

#endif /* JOS_KERN_SCHED_H */
