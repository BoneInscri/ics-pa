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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum
{
  TYPE_I,
  TYPE_U,
  TYPE_S,
  TYPE_J,
  TYPE_R,
  TYPE_B,
  TYPE_N, // none
};

#define src1R()     \
  do                \
  {                 \
    *src1 = R(rs1); \
  } while (0)
#define src2R()     \
  do                \
  {                 \
    *src2 = R(rs2); \
  } while (0)

#define immI()                        \
  do                                  \
  {                                   \
    *imm = SEXT(BITS(i, 31, 20), 12); \
  } while (0)
#define immU()                              \
  do                                        \
  {                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
  } while (0)
#define immS()                                               \
  do                                                         \
  {                                                          \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); \
  } while (0)
#define immJ()                                                                                                            \
  do                                                                                                                      \
  {                                                                                                                       \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1); \
  } while (0)

#define immB()                                                                                                        \
  do                                                                                                                  \
  {                                                                                                                   \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); \
  } while (0)

// 寄存器编号
int rs1;
int rs2;

// printf("jar offset : %lx\n", *imm);
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type)
{
  uint32_t i = s->isa.inst.val;
  rs1 = BITS(i, 19, 15);
  rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  switch (type)
  {
  case TYPE_I:
    src1R();
    immI();
    // printf("type_I, imm : %lx\n", *imm);
    break;
  case TYPE_U:
    immU();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;
  case TYPE_J:
    immJ();
    break;
  case TYPE_R:
    src1R();
    src2R();
    // printf("add, src1 : %lx, src2 : %lx, result : %lx \n", *src1, *src2, *src1 + *src2);
    break;
  case TYPE_B:
    src1R();
    src2R();
    immB(); // don't forget it!
    // printf("type_B, offset : %lx\n", *imm);
    break;
  }
}
void Call_print(vaddr_t caller_pc, vaddr_t callee_pc);
void Ret_print(vaddr_t ret_from_pc, vaddr_t ret_to_pc);

static int decode_exec(Decode *s)
{
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
  // printf("cpu.pc : %lx, spc : %lx, snpc : %lx, dnpc : %lx\n", cpu.pc, s->pc, s->snpc, s->dnpc);

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)         \
  {                                                                  \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__;                                                     \
  }
  INSTPAT_START();

  // ================ U-type ======================
  INSTPAT("???????????????????? ????? 0110111", li, U, R(rd) = imm);            // load upper 20bits
  INSTPAT("???????????????????? ????? 0010111", auipc, U, R(rd) = s->pc + imm); // add

  // ================ I-type ======================
  INSTPAT("???????????? ????? 000 ????? 0010011", addi, I, R(rd) = src1 + imm); // +
  INSTPAT("???????????? ????? 111 ????? 0010011", andi, I, R(rd) = src1 & imm); // &
  INSTPAT("???????????? ????? 100 ????? 0010011", xori, I, R(rd) = src1 ^ imm); // xor

  INSTPAT("???????????? ????? 011 ????? 0000011", ld, I, R(rd) = Mr(src1 + imm, 8));           // 64bits
  INSTPAT("???????????? ????? 010 ????? 0000011", lw, I, R(rd) = SEXT(Mr(src1 + imm, 4), 32)); // 32bits sign-extend
  INSTPAT("???????????? ????? 001 ????? 0000011", lh, I, R(rd) = SEXT(Mr(src1 + imm, 2), 16)); // 16bits sign-extend

  INSTPAT("???????????? ????? 100 ????? 0000011", lbu, I, R(rd) = Mr(src1 + imm, 1)); // 8bits zero-extend
  INSTPAT("???????????? ????? 101 ????? 0000011", lhu, I, R(rd) = Mr(src1 + imm, 2)); // 16bits zero-extend

  // INSTPAT("???????????? ????? 000 ????? 1100111", jalr, I, R(rd) = s->snpc; s->dnpc = ((src1 + imm) & ~1); Ret_print(s->pc, s->dnpc));
  // jalr 既可以充当call 的作用，也可以充当ret 的作用
  // 如果rd是 ra寄存器，说明是call指令，如果rs1是 ra寄存器，说明是ret指令
  INSTPAT("???????????? ????? 000 ????? 1100111", jalr, I, R(rd) = s->snpc; s->dnpc = ((src1 + imm) & ~1); if (rd == 1) Call_print(s->pc, s->dnpc); if (rs1 == 1) Ret_print(s->pc, s->dnpc));
  // src1 和 imm 都看作是无符号数！
  INSTPAT("???????????? ????? 011 ????? 0010011", sltiu, I, R(rd) = (src1 < imm ? 1 : 0)); // set less than imm
  INSTPAT("???????????? ????? 000 ????? 0011011", addiw, I, R(rd) = SEXT(BITS(src1 + imm, 31, 0), 32));
  // 先相加，取低32位 对低32位进行符号扩展
  // sign-extend!!!!!
  // don't copy-paste!!!!!!!!!!!!

  // 必须要进行符号扩展

  INSTPAT("000000 ?????? ????? 001 ????? 0010011", slli, I, R(rd) = (src1 << BITS(imm, 5, 0)));         // 逻辑左移
  INSTPAT("000000 ?????? ????? 101 ????? 0010011", srli, I, R(rd) = (src1 >> BITS(imm, 5, 0)));         // 逻辑右移
  INSTPAT("010000 ?????? ????? 101 ????? 0010011", srai, I, R(rd) = SEXT(src1, 64) >> BITS(imm, 5, 0)); // 算术右移（先符号扩展，然后右移）

  INSTPAT("0000000 ????? ????? 101 ????? 0011011", srliw, I, R(rd) = SEXT(BITS(src1, 31, 0) >> BITS(imm, 4, 0), 32));           // 逻辑右移（低32bits运算，结果需要符号扩展)
  INSTPAT("0000000 ????? ????? 001 ????? 0011011", slliw, I, R(rd) = SEXT(BITS(src1, 31, 0) << BITS(imm, 4, 0), 32));           // 逻辑左移（低32bits运算，结果需要符号扩展)
  INSTPAT("0100000 ????? ????? 101 ????? 0011011", sraiw, I, R(rd) = SEXT(SEXT(BITS(src1, 31, 0), 32) >> BITS(imm, 4, 0), 32)); // 算术右移（低32bits运算，结果需要符号扩展)

  // srliw 、 slliw 和 sraiw 需要注意 符号扩展和移位的先后顺序！
  // w 结尾的移位操作是 7+5 的模式
  // i 结尾的移位操作是 6+6 的模式

  // ================ S-type ======================
  INSTPAT("??????? ????? ????? 011 ????? 0100011", sd, S, Mw(src1 + imm, 8, src2)); // 64bits
  INSTPAT("??????? ????? ????? 010 ????? 0100011", sw, S, Mw(src1 + imm, 4, src2)); // 32bits
  INSTPAT("??????? ????? ????? 001 ????? 0100011", sh, S, Mw(src1 + imm, 2, src2)); // 16bits
  INSTPAT("??????? ????? ????? 000 ????? 0100011", sb, S, Mw(src1 + imm, 1, src2)); // 8bits

  // ================ J-type ======================
  INSTPAT("???????????????????? ????? 1101111", jal, J, R(rd) = s->snpc; s->dnpc = s->pc + imm; if (rd == 1) Call_print(s->pc, s->dnpc);); // 无条件跳转
  // INSTPAT("???????????????????? ????? 1101111", jal, J, R(rd) = s->snpc; s->dnpc = s->pc + imm; ); // 无条件跳转
  // in fact , imm ==> offset
  // 就是当前指令的地址 + offset

  // ================ R-type ======================
  INSTPAT("0000000 ????? ????? 000 ????? 0110011", add, R, R(rd) = src1 + src2); // +
  INSTPAT("0100000 ????? ????? 000 ????? 0110011", sub, R, R(rd) = src1 - src2); // -
  INSTPAT("0000001 ????? ????? 000 ????? 0110011", mul, R, R(rd) = src1 * src2); // *
  INSTPAT("0000001 ????? ????? 110 ????? 0110011", rem, R, R(rd) = src1 % src2); // %

  INSTPAT("0000000 ????? ????? 111 ????? 0110011", and, R, R(rd) = src1 & src2); // &
  INSTPAT("0000000 ????? ????? 110 ????? 0110011", or, R, R(rd) = src1 | src2);  // |
  INSTPAT("0000000 ????? ????? 100 ????? 0110011", xor, R, R(rd) = src1 ^ src2); // xor

  INSTPAT("0000000 ????? ????? 000 ????? 0111011", addw, R, R(rd) = SEXT(BITS(src1, 31, 0) + BITS(src2, 31, 0), 32)); // +
  INSTPAT("0100000 ????? ????? 000 ????? 0111011", subw, R, R(rd) = SEXT(BITS(src1, 31, 0) - BITS(src2, 31, 0), 32)); // -
  INSTPAT("0000001 ????? ????? 000 ????? 0111011", mulw, R, R(rd) = SEXT(BITS(src1, 31, 0) * BITS(src2, 31, 0), 32)); // *
  INSTPAT("0000001 ????? ????? 100 ????? 0111011", divw, R, R(rd) = SEXT(BITS(src1, 31, 0) / BITS(src2, 31, 0), 32)); // /
  INSTPAT("0000001 ????? ????? 110 ????? 0111011", divw, R, R(rd) = SEXT(BITS(src1, 31, 0) % BITS(src2, 31, 0), 32)); // %

  INSTPAT("0000000 ????? ????? 001 ????? 0111011", sllw, R, R(rd) = SEXT(BITS(src1, 31, 0) << BITS(src2, 4, 0), 32));           // << ll
  INSTPAT("0000000 ????? ????? 101 ????? 0111011", srlw, R, R(rd) = SEXT(BITS(src1, 31, 0) >> BITS(src2, 4, 0), 32));           // >> rl
  INSTPAT("0100000 ????? ????? 101 ????? 0111011", sraw, R, R(rd) = SEXT(SEXT(BITS(src1, 31, 0), 32) >> BITS(src2, 4, 0), 32)); // >> ra

  // 必须要之后进行符号扩展！
  INSTPAT("0000000 ????? ????? 010 ????? 0110011", slt, R, R(rd) = ((sword_t)src1 < (sword_t)src2 ? 1 : 0));
  INSTPAT("0000000 ????? ????? 011 ????? 0110011", sltu, R, R(rd) = (src1 < src2 ? 1 : 0));

  // ================ B-type ======================
  // in fact , imm ==> offset
  INSTPAT(
      "??????? ????? ????? 000 ????? 1100011", beq, B, if (src1 == src2) { s->dnpc = s->pc + imm; }); // ==
  INSTPAT(
      "??????? ????? ????? 001 ????? 1100011", bne, B, if (src1 != src2) { s->dnpc = s->pc + imm; }); // !=
  INSTPAT(
      "??????? ????? ????? 101 ????? 1100011", bge, B, if ((sword_t)src1 >= (sword_t)src2) { s->dnpc = s->pc + imm; }); // >= (signed)
  INSTPAT(
      "??????? ????? ????? 111 ????? 1100011", bgeu, B, if (src1 >= src2) { s->dnpc = s->pc + imm; }); // >= (unsigned)
  INSTPAT(
      "??????? ????? ????? 100 ????? 1100011", blt, B, if ((sword_t)src1 < (sword_t)src2) { s->dnpc = s->pc + imm; }); // < (signed)
  INSTPAT(
      "??????? ????? ????? 110 ????? 1100011", bltu, B, if (src1 < src2) { s->dnpc = s->pc + imm; }); // < (unsigned)
  // 就是当前指令的地址 + offset

  // 其他
  INSTPAT("000000000001 00000 000 00000 1110011", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  // 非法
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s)
{
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
