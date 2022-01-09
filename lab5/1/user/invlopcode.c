// causes an invalid opcode exception

void umain(int argc, char **argv) { asm volatile("ud2"); }