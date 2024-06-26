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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

extern int trace_watchpoint();

#ifdef CONFIG_ITRACE
char logbuf[ITRACE_SIZE][128];
int itrace_p;

__attribute__((used)) static void recordItrace(Decode *s)
{
  char *p = logbuf[itrace_p];
  p += snprintf(p, sizeof(logbuf[0]), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i--)
  {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, logbuf[itrace_p] + sizeof(logbuf[itrace_p]) - p,
              MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif

}
#endif

static void trace_and_difftest(Decode *_this, vaddr_t dnpc)
{
#ifdef CONFIG_ITRACE
  recordItrace(_this);

  if (g_print_step)
  {
    puts(logbuf[itrace_p]);
  }
  itrace_p++;
  if (itrace_p == ITRACE_SIZE)
  {
    itrace_p = 0;
  }

#endif

  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_WATCHPOINT
  if(trace_watchpoint()) {
    nemu_state.state = NEMU_STOP;
  }
#endif
}

//  取指, 译码, 执行, 更新PC
static void exec_once(Decode *s, vaddr_t pc)
{
  // printf("=============== pc : %lx ==============\n", cpu.pc);
  s->pc = pc;
  // 当前指令的地址
  s->snpc = pc;
  // 下一条指令的地址 static next PC
  isa_exec_once(s);
  cpu.pc = s->dnpc;
  // printf("hhh : cpu.pc : %lx, dnpc : %lx\n", cpu.pc, s->dnpc);
  // 更新pc为 下一条指令的地址
  // dynamic next PC
}

static void execute(uint64_t n)
{
  Decode s;

  // IFDEF(CONFIG_ITRACE, itrace_p = 0);
  for (; n > 0; n--)
  {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++;
    // 执行的指令数量+1
    trace_and_difftest(&s, cpu.pc);

    if (nemu_state.state != NEMU_RUNNING)
    {
      // NEMU表示这个模拟的计算机，只有在RUNNING的状态下才能继续执行指令！
      break;
    }
    // printf("%d\n", nemu_state.state);
    IFDEF(CONFIG_DEVICE, device_update());
    // cpu_exec()在执行每条指令之后就会调用device_update()函数
  }
}

static void statistic()
{
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg()
{
  isa_reg_display();
  statistic();
}

void itrace_print()
{
#ifdef CONFIG_ITRACE
  for (int i = 0; i < ITRACE_SIZE; i++)
  {
    if (logbuf[i])
    {
      if (i != itrace_p)
      {
        log_write("    ");
      }
      else
      {
        log_write("--> ");
      }
      log_write("%s\n", logbuf[i]);
    }
  }
#endif
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  g_print_step = (n < MAX_INST_TO_PRINT);
  // g_print_step = true;
  switch (nemu_state.state)
  {
  case NEMU_END:
  case NEMU_ABORT:
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  default:
    nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state)
  {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    break;
  case NEMU_END:
  case NEMU_ABORT:
    itrace_print();
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc);
    // fall through
  case NEMU_QUIT:
    statistic();
  }
}
