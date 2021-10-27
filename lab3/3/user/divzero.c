// causes a divide by zero exception

void umain(int argc, char *argv) {
  int zero = 0;
  int x = 1 / zero;
}