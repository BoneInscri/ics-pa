#include <common.h>
#include "syscall.h"
#include <fs.h>

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

static const char *flags_2_str[] = {
    [SEEK_CUR] = "seek_cur",
    [SEEK_END] = "seek_end",
    [SEEK_SET] = "seek_set"};

#endif

#define STDIN 0
#define STDOUT 1
#define STDERROR 2

typedef struct
{
  uint64_t sec;
  uint64_t usec;
} timeval;

static void do_yield(Context *c)
{
  c->RETVAL = 0;
}

static void do_exit(Context *c)
{
  halt(c->ARG1);
}

static void do_write(Context *c)
{
  int fd = c->ARG1;
  void *buf = (void *)(c->ARG2);
  size_t count = c->ARG3;
  assert(buf);
  assert(count > 0);
  c->RETVAL = fs_write(fd, buf, count);
}

uintptr_t brk = 0;

static void do_brk(Context *c)
{
  assert(brk != 0);
  // uintptr_t brk_old = brk;
  intptr_t increment = c->ARG1;
  increment += 4096 * 20;
  brk += increment;
  // panic("brk error");
  // c->RETVAL = 0;
  c->RETVAL = brk;
}

static void do_open(Context *c)
{
  char *filename = (char *)c->ARG1;
  assert(filename);

  int fd = fs_open(filename, 0, 0);

  c->RETVAL = fd;
}

static void do_lseek(Context *c)
{
  int fd = c->ARG1;
  assert(fd >= 0);
  size_t offset = c->ARG2;
  int whence = c->ARG3;

  c->RETVAL = fs_lseek(fd, offset, whence);
}

static void do_read(Context *c)
{
  int fd = c->ARG1;
  void *buf = (void *)c->ARG2;
  size_t count = c->ARG3;

  c->RETVAL = fs_read(fd, buf, count);
}

static void do_close(Context *c)
{
  int fd = c->ARG1;
  c->RETVAL = fs_close(fd);
}

static void do_gettimeofday(Context *c)
{
  timeval *tv = (timeval *)c->ARG1;
  // timezione *tz = (struct timezione *)c->ARG2;
  // assert(tz == NULL);
  AM_TIMER_UPTIME_T uptime;
  uptime = io_read(AM_TIMER_UPTIME);

  tv->sec = uptime.us / 1000000;
  tv->usec = uptime.us % 1000000;

  c->RETVAL = 0;
}

static void (*syscall_funcs[])(Context *) = {
    [SYS_exit] = do_exit,
    [SYS_yield] = do_yield,
    [SYS_write] = do_write,
    [SYS_brk] = do_brk,
    [SYS_open] = do_open,
    [SYS_lseek] = do_lseek,
    [SYS_read] = do_read,
    [SYS_close] = do_close,
    [SYS_gettimeofday] = do_gettimeofday};

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define N_SYSCALL ARRLEN(syscall_funcs)

void do_syscall(Context *c)
{
  uintptr_t sys_num = c->SYSNUM;

#ifdef __STRACE__
  printf("syscall num : %d, name : %s\n", sys_num, sysnum_2_sysname[sys_num]);
  if (sys_num == SYS_open)
  {
    printf("arg1 : %s\n", c->ARG1);
  }
  if (sys_num == SYS_lseek)
  {
    printf("arg3 : %s\n", flags_2_str[c->ARG3]);
  }
  printf("arg1 : %lx, arg2 : %lx, arg3 : %lx\n", c->ARG1, c->ARG2, c->ARG3);
#endif

  int find = 0;
  for (int num = 0; num < N_SYSCALL; num++)
  {
    if (sys_num == num && syscall_funcs[num])
    {
      syscall_funcs[sys_num](c);
      find = 1;
    }
  }
  if (!find)
    panic("Unhandled syscall ID = %d", sys_num);

#ifdef __STRACE__
  printf("retval : %ld %lx\n\n", c->RETVAL, c->RETVAL);
#endif
}
