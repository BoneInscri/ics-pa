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

void sdb_mainloop();

#ifdef CONFIG_RECURSIVE_TEST
void engine_start() {
  cpu_exec(-1);
}

#else
void engine_start() {
#ifdef CONFIG_TARGET_AM
// #ifdef CONFIG_RUN_DIRECTLY_COND
  cpu_exec(-1);
#else
  // cpu_exec(-1);
  /* Receive commands from user. */
  sdb_mainloop();
#endif
}
#endif
