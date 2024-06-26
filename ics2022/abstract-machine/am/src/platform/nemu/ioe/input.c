#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t _kbd = inl(KBD_ADDR);
  kbd->keydown = (_kbd & KEYDOWN_MASK ? true : false);
  kbd->keycode = _kbd & ~KEYDOWN_MASK;
}
