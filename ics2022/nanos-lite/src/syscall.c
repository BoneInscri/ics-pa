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

void do_syscall(Context *c)
{
  uintptr_t sys_num = c->SYSNUM;
#ifdef __STRACE__
  printf("syscall num : %d, name : %s\n", sys_num, sysnum_2_sysname[sys_num]);
#endif

  switch (sys_num)
  {
  case SYS_yield:
    c->RETVAL = 0;
    break;
  case SYS_exit:
    halt(c->ARG1);
    break;
  default:
    panic("Unhandled syscall ID = %d", sys_num);
  }
}
