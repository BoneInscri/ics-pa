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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
// #include "memory/paddr.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
  // if(args == NULL) {
  //   goto fail;
  // }
  uint64_t N = 1;

  // get N
  if (args != NULL)
    sscanf(args, "%ld", &N);

  // execute N steps
  cpu_exec(N);

  return 0;
// fail:
//   printf("help : si [N]\n");
//   return 0;
}

static int cmd_info(char *args)
{
  char SUBCMD;
  if (args == NULL)
  {
    goto fail;
  }
  sscanf(args, "%c", &SUBCMD);
  switch (SUBCMD)
  {
  case 'r':
    isa_reg_display();
    break;
  case 'w':
    show_wp();
    // panic("Not implemented\n");
    break;
  default:
    panic("cmd_info : Not implemented\n");
    break;
  }

  return 0;
fail:
    printf("help : info r/w\n");
    return 0;
}

extern int valid;

static int cmd_p(char *args)
{
  if (args == NULL)
  {
    goto fail;
  }
  bool success = true;

  word_t EXPR = expr(args, &success);
  // if (success && valid)
  if (success && valid)
    printf("0x%lx %ld\n", EXPR, EXPR);
  // else
  // panic("bad expression\n");

  return 0;
fail:
  printf("help : p [EXPR]\nexample : p $eax + 1\n");
  return 0;
}

static int cmd_x(char *args)
{
  if (args == NULL)
  {
    goto fail;
  }
  uint64_t N;
  if (sscanf(args, "%ld ", &N) != 1)
  {
    goto fail;
  }
  char *N_str = strtok(args, " ");
  char *EXPR_str = args + strlen(N_str) + 1;

  // ==== for test ====
  // uint32_t result;
  // if (sscanf(args, "%d ", &result) != 1)
  // {
  //   goto fail;
  // }
  // char *result_str = strtok(args, " ");
  // char *EXPR_str = args + strlen(result_str) + 1;

  bool success = true;
  // printf("EXPR_str : %s\n", EXPR_str);
  paddr_t EXPR = expr(EXPR_str, &success);

  if (!valid && success)
  {
    // printf("result : %d, EXPR : %d, success : %d\n", result, EXPR, success);
    panic("bad expression\n");
  } else if(!success) {
    return 0;
  }
  // printf("N : %ld, EXPR : %d, success : %d\n", N, EXPR, success);
  // if(result != EXPR) {
  // printf("result : %d, EXPR : %d, success : %d\n", result, EXPR, success);
  // panic("test fail\n");
  // }
  // return 0;
  // // ========
  // panic("no way\n");
  // if (sscanf(args, "%ld %x", &N, &EXPR) != 2)
  // {
  //   goto fail;
  // }

  paddr_t addr_p = 0;
  for (int i = 0; i < N * 4; i++)
  {
    // 16 个字节一行
    if (addr_p % 16 == 0)
    {
      printf("0x%x:", addr_p + EXPR);
    }
    word_t read_data = vaddr_read(EXPR + i, 1);
    printf(" %02lx", read_data);
    addr_p++;
    if (addr_p % 16 == 0)
    {
      printf("\n");
    }
  }
  printf("\n");
  return 0;

fail:
  printf("help : x [N] [EXPR]\n");
  return 0;
}

static int cmd_w(char *args)
{
  if (args == NULL)
  {
    goto fail;
  }
  bool success = true;
  word_t exp_val = expr(args, &success);
  if (!valid && success)
  {
    printf("bad expression : %s\n", args);
    goto fail;
    // panic("bad expression\n");
  } else if(!success) {
    return 0;
  }

  WP* wp = new_wp(args, exp_val);
  printf("Watchpoint %d: %s\n", wp->NO, args);

  return 0;
fail:
  printf("help : w [N] [EXPR]\n");
  return 0;
}

static int cmd_d(char *args)
{
  if (args == NULL)
  {
    goto fail;
  }  
  int N;
  sscanf(args, "%d", &N);
  int ret = delete_wp(N);
  if(ret < 0) {
    printf("No breakpoint number %d\n", N);
  }
  return 0;

fail:
  printf("help : d [N]\n");
  return 0;
}

#ifdef CONFIG_ITRACE
extern char logbuf[ITRACE_SIZE][128];
extern int itrace_p;

static int cmd_itrace(char *args)
{
  itrace_print();
  return 0;
}
#endif

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Single Instruction", cmd_si},
    {"info", "State of program", cmd_info},
    {"x", "Memory scan", cmd_x},
    {"p", "Caculate value of expression", cmd_p},
    {"w", "Set Watchpoints", cmd_w},
    {"d", "Delete Watchpoints", cmd_d}, 
#ifdef CONFIG_ITRACE
    {"itrace", "Print the instructions ring buffer", cmd_itrace}
#endif
    /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }
    
    if(args!=NULL)
    assert(strlen(args) <= cmd_arg_max);

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (!strcmp(cmd, cmd_table[i].name))
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
