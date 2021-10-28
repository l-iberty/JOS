#ifndef JOS_INC_SYSCALL_H
#define JOS_INC_SYSCALL_H

/* system call numbers */
enum {
  SYS_puts = 0,
  SYS_getc,
  SYS_getenvid,
  SYS_env_destroy,
  NSYSCALLS
};

#endif /* JOS_INC_SYSCALL_H */
