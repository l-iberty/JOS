// faults with a read from kernel space

void umain(int argc, char **argv) { int x = *(int *)0xf0100000; }