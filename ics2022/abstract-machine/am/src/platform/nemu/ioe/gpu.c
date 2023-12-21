#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

// #define MODE_800x600
#ifdef MODE_800x600
#define W 800
#define H 600
#else
#define W 400
#define H 300
#endif

void __am_gpu_init()
{
  int i;
  int w = W;
  int h = H;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i++)
    fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg)
{
  *cfg = (AM_GPU_CONFIG_T){
      .present = true, .has_accel = false, .width = W, .height = H, .vmemsz = 0};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl)
{
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  // if (w == 0 || h == 0)
  // {
  //   goto finish;
  // }
  // if (x >= H || y >= W)
  // {
  //   goto finish;
  // }
  // printf("%d %d %d %d\n", x, y, w, h);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;

  int pixel_idx = 0;

  for (int j = y; j < y + h; j++)
  {
    for (int i = x; i < x + w; i++)
    {
      fb[j * W + i] = ((uint32_t *)ctl->pixels)[pixel_idx++];
      // printf("fb pos %d, pixels pos %d color : %x\n", j * W + i, pixel_idx - 1, fb[j * W + i]);
    }
  }
// finish:
  if (ctl->sync)
  {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status)
{
  status->ready = true;
}
