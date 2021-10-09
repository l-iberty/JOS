extern void __lib_putc();

void bootmain() {
  __lib_putc();
  for (;;)
    ;
}