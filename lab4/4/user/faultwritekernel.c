// faults with a write to kernel space

void umain(int argc, char **argv) { *(int *)0xf0100000 = 0; }