// causes a breakpoint trap

void umain(int argc, char **argv) {
  int i;
  asm volatile("int $3");
  i = 0;
  i = 0;
  i = 0;
  i = 0;
  i = 0;
  asm volatile("int $3");
}