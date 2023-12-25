#include <common.h>
#include "syscall.h"
enum
{
  SYS_exit,
  SYS_yield,
  SYS_open,
  SYS_read,
  SYS_write,
  SYS_kill,
  SYS_getpid,
  SYS_close,
  SYS_lseek,
  SYS_brk,
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};

#ifdef __STRACE__
static const char *sysnum_2_sysname[] = {
    [SYS_exit] = "exit",
    [SYS_yield] = "yield",
    [SYS_open] = "open",
    [SYS_read] = "read",
    [SYS_write] = "write",
    [SYS_kill] = "kill",
    [SYS_getpid] = "getpid",
    [SYS_close] = "close",
    [SYS_lseek] = "lseek",
    [SYS_brk] = "brk",
    [SYS_fstat] = "fstat",
    [SYS_time] = "time",
    [SYS_signal] = "signal",
    [SYS_execve] = "execve",
    [SYS_fork] = "fork",
    [SYS_link] = "link",
    [SYS_unlink] = "unlink",
    [SYS_wait] = "wait",
    [SYS_times] = "times",
    [SYS_gettimeofday] = "gettimeofday"};
#endif

#define STDIN 0
#define STDOUT 1
#define STDERROR 2

static void do_yield(Context *c)
{
  c->RETVAL = 0;
}

static void do_exit(Context *c)
{
  halt(c->ARG1);
}

// ssize_t write(int fd, const void *buf, size_t count);
static void do_write(Context *c)
{
  int fd = c->ARG1;
  char *buf = (char *)(c->ARG2);
  size_t count = c->ARG3;
  assert(fd == STDOUT || fd == STDERROR);
  assert(buf);
  assert(count > 0);
  for (int i = 0; i < count; i++)
  {
    putch(buf[i]);
  }
  c->RETVAL = count;
}

// static void printf_buf(char *s, int len)
// {
//   for (int i = 0; i < len; i++)
//   {
//     putch(s[i]);
//   }
// }

uintptr_t brk = 0;

static void do_brk(Context *c)
{
  assert(brk != 0);
  // uintptr_t brk_old = brk;
  intptr_t increment = c->ARG1; 
  brk += increment;
  // panic("brk error");
  // c->RETVAL = 0;
  c->RETVAL = brk;
}

static void (*syscall_funcs[])(Context *) = {
    [SYS_exit] = do_exit,
    [SYS_yield] = do_yield,
    [SYS_write] = do_write,
    [SYS_brk] = do_brk};

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define N_SYSCALL ARRLEN(syscall_funcs)

void do_syscall(Context *c)
{
  uintptr_t sys_num = c->SYSNUM;
#ifdef __STRACE__
  printf("syscall num : %d, name : %s\n", sys_num, sysnum_2_sysname[sys_num]);
  printf("arg1 : %lx, arg2 : %lx, arg3 : %lx\n", c->ARG1, c->ARG2, c->ARG3);
#endif

  int find = 0;
  for (int num = 0; num < N_SYSCALL; num++)
  {
    if (sys_num == num)
    {
      syscall_funcs[sys_num](c);
      find = 1;
    }
  }
  if (!find)
    panic("Unhandled syscall ID = %d", sys_num);
}
