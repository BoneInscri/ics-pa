#include <common.h>
#include "syscall.h"
enum {
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

void do_syscall(Context *c) {
  uintptr_t sys_num = c->SYSNUM;

  switch (sys_num) {
    case SYS_yield:
      c->RETVAL = 0;
      break;
    case SYS_exit:
      halt(c->ARG1);
      break;
    default: panic("Unhandled syscall ID = %d", sys_num);
  }
}
