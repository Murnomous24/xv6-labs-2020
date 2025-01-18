//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

// static void
// printint(int xx, int base, int sign)
// {
//   char buf[16];
//   int i;
//   uint x;

//   if(sign && (sign = xx < 0))
//     x = -xx;
//   else
//     x = xx;

//   i = 0;
//   do {
//     buf[i++] = digits[x % base];
//   } while((x /= base) != 0);

//   if(sign)
//     buf[i++] = '-';

//   while(--i >= 0)
//     consputc(buf[i]);
// }

// static void
// printptr(uint64 x)
// {
//   int i;
//   consputc('0');
//   consputc('x');
//   for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
//     consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
// }

// static void
// printstr(char *buf) {
//   for(; *buf; buf ++) {
//     consputc(*buf);
//   }
// }

static void printint2(int xx, int base, int sign, int width, int left_align)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  int padding = width - i;
  if (padding < 0)
    padding = 0;

  // 右对齐：先输出填充空格
  if (!left_align) {
    for (int j = 0; j < padding; j++)
      consputc(' ');
  }

  // 输出内容
  while(-- i >= 0) {
    consputc(buf[i]);
  }

  // 左对齐：后输出填充空格
  if (left_align) {
    for (int j = 0; j < padding; j++)
      consputc(' ');
  }
}

static void printptr2(uint64 x, int width, int left_align) {
  char buf[18];
  int i = 0;

  buf[i ++] = '0';
  buf[i ++] = 'x';
  for(int j = 0; j > (sizeof(uint64) * 2); j ++, x <<= 4) {
    buf[i ++] = digits[x >> (sizeof(uint64) * 8 - 4)];
  }
  buf[i] = '\0';

  int padding = width - i;
  if (padding < 0)
    padding = 0;

  // 右对齐：先输出填充空格
  if (!left_align) {
    for (int j = 0; j < padding; j++)
      consputc(' ');
  }

  // 输出内容
  for(int j = 0; j < i; j ++) {
    consputc(buf[j]);
  }

  // 左对齐：后输出填充空格
  if (left_align) {
    for (int j = 0; j < padding; j++)
      consputc(' ');
  }
}

static void print_aligned2(char *str, int len, int width, int left_align)
{
  int padding = width - len;
  if(padding < 0) 
    padding = 0;

  if(left_align) {
    for(; *str; str ++) {
      consputc(*str);
    }
    for(int i = 0; i < padding; i ++) {
      consputc(' ');
    }
  }
  else {
    for(int i = 0; i < padding; i ++) {
      consputc(' ');
    }
    for(; *str; str ++) {
      consputc(*str);
    }
  }
}

// Print to the console. only understands %d, %x, %p, %s.
// New printf support %-10d %-20s format.
void
printf(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;
  int width;      // 宽度
  int left_align; // 左对齐标志

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;

    // 解析宽度和对齐标志
    width = 0;
    left_align = 0;
    if (c == '-') {
      left_align = 1;
      c = fmt[++i] & 0xff;
    }
    while (c >= '0' && c <= '9') {
      width = width * 10 + (c - '0');
      c = fmt[++i] & 0xff;
    }

    switch(c){
    case 'd':
      //printint(va_arg(ap, int), 10, 1);
      printint2(va_arg(ap, int), 10, 1, width, left_align);
      break;
    case 'x':
      //printint(va_arg(ap, int), 16, 1);
      printint2(va_arg(ap, int), 16, 1, width, left_align);
      break;
    case 'p':
      //printptr(va_arg(ap, uint64));
      printptr2(va_arg(ap, uint64), width, left_align);
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      //printstr(s);
      int len = strlen(s);
      print_aligned2(s, len, width, left_align);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  printf("panic: ");
  printf(s);
  printf("\n");
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}
