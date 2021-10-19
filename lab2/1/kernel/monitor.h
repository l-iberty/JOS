#ifndef _JOS_MONITOR_H_
#define _JOS_MONITOR_H_

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_reboot(int argc, char **argv, struct Trapframe *tf);
int mon_showmappings(int argc, char **argv, struct Trapframe *tf);

#endif  // _JOS_MONITOR_H_