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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);
uint32_t eval(int start, int end);


#define ITRACE_SIZE 30
// itrace 环形缓冲区大小

#define NR_WP 64
#define cmd_arg_max 50
// 双向链表(非循环），用数组改造
typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;
  struct watchpoint *prev;
  word_t exp_val;
  char watch_name[cmd_arg_max];
  /* TODO: Add more members if necessary */
} WP;
WP *new_wp(char* exp, word_t exp_val);
int delete_wp(int NO);
void show_wp();
int trace_watchpoint();

#endif
