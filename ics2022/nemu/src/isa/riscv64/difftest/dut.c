/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern CPU_state cpu;
extern const char *regs[];
extern const char *csrs[];
extern const int csrs_idx[];
#define N_REGS 32
#define N_CSRS_VALID 5

static void regs_diff_display(CPU_state *ref_r, vaddr_t pc) {
  if(ref_r->pc!= cpu.pc) {
    printf("nemu : pc             0x%lx   %ld\n", cpu.pc, cpu.pc);
    printf("ref  : pc             0x%lx   %ld\n", ref_r->pc, ref_r->pc);
  }
  for (int i = 0; i < N_REGS; i++) {
    if(gpr(i) != ref_r->gpr[i]) {
      printf("nemu : x%d/%-15s0x%lx   %ld\n", i, regs[i], gpr(i), gpr(i));
      printf("ref  : x%d/%-15s0x%lx   %ld\n", i, regs[i], ref_r->gpr[i], ref_r->gpr[i]);
    }
  }

  // for (int i = 0; i< N_CSRS_VALID; i++) {
  //   int idx = csrs_idx[i];
  //   printf("nemu : %-15s0x%lx   %ld\n", csrs[idx], csr(idx), csr(idx));
  //   printf("ref  : %-15s0x%lx   %ld\n", csrs[idx], ref_r->csr[idx], ref_r->csr[idx]);
  // }
}

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if(ref_r->pc!= cpu.pc) {
    return false;
  }
  for (int i = 0; i < N_REGS; i++) {
    if(gpr(i) != ref_r->gpr[i]) {
      regs_diff_display(ref_r, pc);
      return false;
    }
  }
  // for (int i = 0; i< N_CSRS_VALID; i++) {
  //   int idx = csrs_idx[i];
  //   if(csr(idx) != ref_r->csr[idx]) {
  //     regs_diff_display(ref_r, pc);
  //     return false;
  //   }
  // }
  return true;
}

void isa_difftest_attach() {
}
