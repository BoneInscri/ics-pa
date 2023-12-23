#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context *(*user_handler)(Event, Context *) = NULL;

__attribute__((used)) static void print_context(Context *c)
{
  printf("mcause 0x%lx\n", c->mcause);
  printf("mstatus 0x%lx\n", c->mstatus);
  printf("mepc 0x%lx\n", c->mepc);
  for (int i = 0; i < 32; i++)
  {
    printf("x%d%10x\n", i, c->gpr[i]);
  }
}

Context *__am_irq_handle(Context *c)
{
  // print_context(c);
  if (user_handler)
  {
    Event ev = {0};
    switch (c->mcause)
    {
    case M_SYSCALL:
      ev.event = EVENT_YIELD;
      c->mepc += 4;
      printf("triger a yield event, mepc : %lx, a0 = %d\n", c->mepc, c->gpr[10]);
      break;
    default:
      ev.event = EVENT_ERROR;
      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *))
{
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  // ret the status of cpu
  asm volatile("csrw mstatus, %0" : : "r"(0xa00001800));

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg)
{
  return NULL;
}

// 进行自陷操作
void yield()
{
  asm volatile("li a7, -1; ecall");
}

bool ienabled()
{
  return false;
}

void iset(bool enable)
{
}
