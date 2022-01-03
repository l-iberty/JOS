// Fork a binary tree of processes and display their structure.

#include <include/lib.h>

#define DEPTH 3

void forktree(const char *cur);

void forkchild(const char *cur, char branch) {
  char nxt[DEPTH + 1];

  if (strlen(cur) >= DEPTH) return;

  sprintf(nxt, "%s%c", cur, branch);
  if (sfork() == 0) {
    forktree(nxt);
    exit();
  }
}

void forktree(const char *cur) {
  printf("%04x: I am '%s'\n", sys_getenvid(), cur);

  forkchild(cur, '0');
  forkchild(cur, '1');
}

void umain(int argc, char **argv) { forktree(""); }
