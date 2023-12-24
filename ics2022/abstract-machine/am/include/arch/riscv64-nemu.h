#ifndef ARCH_H__
#define ARCH_H__

struct Context
{
  // fix the order of these members to match trap.S
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define SYSNUM gpr[17] // a7
#define RETVAL gpr[10] // a0
#define ARG1 gpr[10]   // a0
#define ARG2 gpr[11]   // a1
#define ARG3 gpr[12]   // a2
#define ARG4 gpr[13]   // a3
#define ARG5 gpr[14]   // a4
#define ARG6 gpr[15]   // a5
#define ARG7 gpr[16]   // a6

#endif
