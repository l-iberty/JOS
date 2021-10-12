#ifndef _JOS_CONSOLE_H_
#define _JOS_CONSOLE_H_

#define CGA_BASE 0xB8000
#define CGA_SIZE 0x8000  /* CGA彩色字符模式显存空间: 0xB8000 ~ 0xBFFFF */

#define CGA_ATTR_BLACK     0x0
#define CGA_ATTR_WHITE     0xf
#define CGA_ATTR_LIGHTCYAN 0xb
#define CGA_ATTR_LIGHTGRAY 0x7
#define CGA_ATTR_LIGHTRED  0xc

#define CRT_ROWS 25
#define CRT_COLS 80
#define CRT_SIZE (CRT_ROWS * CRT_COLS)

#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5
#define CRT_CURSOR_HIGH 14
#define CRT_CURSOR_LOW  15

void cons_init();
void cons_print(const char* s);

#endif /* _JOS_CONSOLE_H_ */