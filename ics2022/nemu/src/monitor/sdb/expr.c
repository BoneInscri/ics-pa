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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <stdint.h>
// #include <memory/paddr.h>
#include <memory/vaddr.h>

enum
{
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NOT_EQ,
  TK_NUM_DEC,
  TK_U,
  TK_NUM_HEX,
  TK_NUM_REG,
  TK_AND_LOG,
  TK_DEREF, // 和乘法区分开，解引用
  TK_NEG,
  TK_MAX // used for priority

  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {
    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},                    // spaces
    {"[1-9]+[0-9]*", TK_NUM_DEC},         // 十进制整数
    {"\\+", '+'},                         // plus
    {"\\-", '-'},                         // minus
    {"\\/", '/'},                         // division
    {"\\*", '*'},                         // multiple
    {"\\(", '('},                         // left bracket
    {"\\)", ')'},                         // right bracket
    {"[0][x|X][0-9a-z]+[0-9a-z]*", TK_NUM_HEX}, // 十六进制整数
    {"[$][a-z0-9]+", TK_NUM_REG},         // 寄存器数
    {"[&][&]", TK_AND_LOG},               // 逻辑与
    {"[!][=]", TK_NOT_EQ},                // 不等于号
    {"\\U", TK_U},                        // 'U'
    {"==", TK_EQ},                        // equal
};

#define NR_REGEX ARRLEN(rules)

#define BASE_OP(type) ((type == '+') || (type == '-') || (type == '*') || (type == '/'))// + - * /
#define COMP_OP(type) ((type == TK_EQ) || (type == TK_NOT_EQ)) // == and !=
#define LOG_OP(type) ((type == TK_AND_LOG))                    // &&
#define UNARY_OP(type) ((type == TK_DEREF) || (type == TK_NEG)) // 一元运算符
#define TWO_OP(type) (BASE_OP(type) || COMP_OP(type) || LOG_OP(type))// 二元运算符
#define OP(type) (UNARY_OP(type) || TWO_OP(type))

#define NUM(type) ((type == TK_NUM_DEC) || (type == TK_NUM_HEX) || (type == TK_NUM_REG))// 十进制 十六进制 寄存器

#define PREV_FOR_DEREF_AND_NEG(type) ((OP(type) || type == '(') && !NUM(type) && type != ')')

static regex_t re[NR_REGEX] = {};
// 优先级
static int priority[TK_MAX] __attribute__((used)) = {};
// debug
// type -> str
static const char *type2str[TK_MAX] __attribute__((used)) = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
  // init the priority of operator
  priority[TK_DEREF] = 2;
  priority[TK_NEG] = 2;
  priority['*'] = 3;
  priority['/'] = 3;
  priority['+'] = 4;
  priority['-'] = 4;
  priority[TK_EQ] = 7;
  priority[TK_NOT_EQ] = 7;
  priority[TK_AND_LOG] = 11;

  type2str[TK_AND_LOG] = "&&";
  type2str[TK_EQ] = "==";
  type2str[TK_NOT_EQ] = "!=";
  type2str[TK_DEREF] = "DEREF";
  type2str[TK_NEG] = "NEG";
}

#define token_str_max 32
#define tokens_max 65536

typedef struct token
{
  int type;
  char str[token_str_max];
} Token;

static Token tokens[tokens_max] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static void strncpy_tmp(char *s, char *d, int len)
{
  for (int i = 0; i < len; i++)
  {
    d[i] = s[i];
  }
  d[len] = '\0';
}
// debug
__attribute__((used)) static void token_print(int i) {
  if (tokens[i].type < 255)
    printf("i : %d, type : %c\n", i, tokens[i].type);
  else if (type2str[tokens[i].type])
    printf("i : %d, type : %s\n", i, type2str[tokens[i].type]);
  else
    printf("i : %d, type : %s\n", i, tokens[i].str);
}
__attribute__((used)) static void tokens_print()
{
  for (int i = 0; i < nr_token; i++)
    token_print(i);
}

int valid;
static bool check_backet_valid(int start, int end)
{
  // the backet is paired ?
  int pair = 0;
  for (int i = start; i <= end; i++)
  {
    if (tokens[i].type == '(')
    {
      pair++;
    }
    else if (tokens[i].type == ')')
    {
      pair--;
      if (pair < 0)
      {
        // left right is ok
        // right left is error
        return false;
      }
    }
  }
  return pair ? false : true;
}
static bool check_parentheses(int start, int end)
{
  // is valid?
  valid = check_backet_valid(start, end);
  if (!valid)
    return false;
  // the expression is be surrounded by () ?
  if (tokens[start].type == '(' && tokens[end].type == ')')
  {
    return check_backet_valid(start + 1, end - 1);
  }
  else
  {
    return false;
  }
}

// 找到主运算符
static int find_main_op(int start, int end)
{
  int in_backet = 0; // 在括号里面？
  int op = -1;
  for (int i = start; i <= end; i++)
  {
    if (tokens[i].type == TK_NUM_DEC)
      continue;
    if (tokens[i].type == '(')
    {
      in_backet++;
    }
    if (tokens[i].type == ')')
    {
      in_backet--;
    }
    // 在括号里面
    if (in_backet)
      continue;

    // 主符号
    if ((tokens[i].type))
    {
      if ((op == -1) || (priority[tokens[i].type] >= priority[tokens[op].type]))
      {
        op = i;
      }
    }
  }
  return op;
}

// DEC、HEX、Register
static word_t GET_NUM(int pos)
{
  word_t num_tmp = 0;
  switch (tokens[pos].type)
  {
  case TK_NUM_DEC:
    sscanf(tokens[pos].str, "%ld", &num_tmp);
    break;
  case TK_NUM_HEX:
    sscanf(tokens[pos].str, "%lx", &num_tmp);
    break;
  case TK_NUM_REG:
    bool success = false;
    num_tmp = isa_reg_str2val(tokens[pos].str, &success);
    if (!success)
    {
      valid = 0;
      // panic("register name error\n");
      printf("register name error\n");
    }
    break;
  default:
    TODO();
  }
  return num_tmp;
}

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        // printf("HIT : %s, %d\n", substr_start, substr_len);
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);
        // printf("%d\n", substr_len);
        position += substr_len;
        assert(substr_len <= token_str_max);
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        // printf("%d\n", rules[i].token_type);
        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
        case TK_U:
          // ignore spaces !
          // ignore unsigned
          break;
        case '+':
        case '-':
        case '/':
        case '*':
        case '(':
        case ')':
          tokens[nr_token++].type = rules[i].token_type;
          // don't need to get str
          break;
        case TK_NUM_DEC:
        case TK_NUM_HEX:
        case TK_NUM_REG:
        case TK_EQ:
        case TK_NOT_EQ:
        case TK_AND_LOG:
          tokens[nr_token].type = rules[i].token_type;
          // a simple method
          strncpy_tmp(substr_start, tokens[nr_token++].str, substr_len);
          break;
        default:
          TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

#define SINGLE_OP(type) ((type == TK_DEREF) || (type == TK_NEG))
word_t eval(int start, int end)
{
  // printf("eval , start : %d, end : %d\n", start, end);
  if (start > end)
  {
    valid = 0;
    /* Bad expression */
    return 0;
  }
  else if (end == start + 1 && SINGLE_OP(tokens[start].type))
  {
    word_t num_tmp = GET_NUM(end);
    switch (tokens[start].type)
    {
    case TK_DEREF:
      return vaddr_read(num_tmp, sizeof(num_tmp));
    case TK_NEG:
      // printf("start : %d, end : %d, TK_NEG\n", start, end);
      return -num_tmp;
    default:
      TODO();
    }
  }
  else if (start == end)
  {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    // word_t num_tmp = 0;
    // sscanf(tokens[start].str, "%ld", &num_tmp);
    // printf("eval , start : %d, end : %d\n", start, end);
    // return num_tmp;
    return GET_NUM(start);
  }
  else if (check_parentheses(start, end) == true)
  {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    // printf("eval , start : %d, end : %d, has backet\n", start, end);
    return eval(start + 1, end - 1);
  }
  else
  {
    if (!valid)
    {
      // printf("eval , start : %d, end : %d, valid == 0\n", start, end);
      return 0;
    }
    /* We should do more things here. */
    int op = find_main_op(start, end);
    // printf("eval , start : %d, end : %d, op = %d\n", start, end, op);
    assert(op != -1);
    // op = the position of 主运算符 in the token expression;
    word_t val1 = eval(start, op - 1);
    word_t val2 = eval(op + 1, end);

    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      return val1 / val2;
    case TK_DEREF:
      return vaddr_read(val2, sizeof(val2));
    case TK_NEG:
      return -val2;
    case TK_EQ:
      return val1 == val2;
    case TK_NOT_EQ:
      return val1 != val2;
    case TK_AND_LOG:
      return val1 && val2;
    default:
      token_print(op);
      assert(0);
    }
  }
}
// error : valid == 0
word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return -1;
  }
  assert(nr_token <= tokens_max);

  for (int i = 0; i < nr_token; i++)
  {
    // 解引用 特判
    if (tokens[i].type == '*' && (i == 0 || PREV_FOR_DEREF_AND_NEG(tokens[i-1].type)))
    {
      tokens[i].type = TK_DEREF;
    }
    // 负号 特判
    if (tokens[i].type == '-' && (i == 0 || PREV_FOR_DEREF_AND_NEG(tokens[i - 1].type)))
    {
      tokens[i].type = TK_NEG;
    }
  }
  // int tmp = check_parentheses(0, nr_token-1);
  // printf("%d\n", tmp);
  // printf("%d\n", nr_token);
  // tokens_print();
  // int tmp = find_main_op(0, nr_token-1);
  // printf("%d\n", tmp);

  /* TODO: Insert codes to evaluate the expression. */
  valid = 1;
  // printf("%d\n", valid);
  return eval(0, nr_token - 1);
}
