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

#include "sdb.h"
#include "isa.h"

static WP wp_pool[NR_WP] = {};
static WP alloc_, free_;

static void init_node(WP *wp)
{
  wp->next = wp;
  wp->prev = wp;
}

static bool is_empty(WP *wp)
{
  return wp->next == wp;
}

static void append_node(WP *head, WP *wp)
{
  wp->next = head;
  wp->prev = head->prev;
  head->prev->next = wp;
  head->prev = wp;
}

static void delete_node(WP *wp)
{
  wp->prev->next = wp->next;
  wp->next->prev = wp->prev;
  wp->next = wp;
  wp->prev = wp;
}

static WP *pop_node(WP *head)
{
  WP *first = head->next;
  delete_node(first);
  return first;
}

void init_wp_pool()
{
  init_node(&alloc_);
  init_node(&free_);

  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    // 链表 连在一起
    // 双向循环！
    // 带头结点！
    append_node(&free_, wp_pool + i);
    wp_pool[i].watch_name[0] = '\0';
  }
}

static void free_wp(WP *wp)
{
  assert(!is_empty(wp));

  delete_node(wp);
  append_node(&free_, wp);
}

WP *new_wp(char* exp, word_t exp_val)
{
  if (is_empty(&free_))
  {
    panic("no more watchpoint\n");
  }

  WP *new_p = pop_node(&free_);
  strcpy(new_p->watch_name, exp);
  append_node(&alloc_, new_p);
  new_p->exp_val = exp_val;
  return new_p;
}

int delete_wp(int NO)
{
  WP *wp = alloc_.next;
  while (wp != &alloc_)
  {
    if (wp->NO == NO)
    {
      free_wp(wp);
      return 0;
    }
    wp = wp->next;
  }
  return -1;
}

void show_wp()
{
  if (is_empty(&alloc_))
  {
    printf("No watchpoints.\n");
    return;
  }
  printf("Num     Type           Disp Enb Address    What     Value\n");

  WP *wp = alloc_.next;
  while (wp != &alloc_)
  {
    printf("%-3d     watchpoint     keep y              %s      0x%lx\n", wp->NO, wp->watch_name, wp->exp_val);
    wp = wp->next;
  }
}

extern int valid;
int trace_watchpoint() {
  int wp_flag = 0;

  WP *wp = alloc_.next;
  while (wp != &alloc_)
  {
    bool success = true;
    word_t EXPR = expr(wp->watch_name, &success);
    if (!valid && success)
    {
      panic("bad expression\n");
    }
    if(wp->exp_val!=EXPR) {
      printf("Error evaluating expression for watchpoint %d\n", wp->NO);
      printf("what : %s, pc : 0x%lx, old_val : 0x%lx, new_val : 0x%lx\n", wp->watch_name, cpu.pc, wp->exp_val, EXPR);
      wp_flag = 1;
      wp->exp_val = EXPR;
      // 我实现的监视点是一直有效，除非使用d [N]命令删除
      // 防止一直提示监视点信息，每次值发生改变后都需要更新！
    }
    wp = wp->next;
  }
  return wp_flag;// 触发监视点？
}