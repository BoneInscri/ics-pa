#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <stdbool.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks()
{
  struct timeval time;
  int ret = gettimeofday(&time, NULL);
  return time.tv_sec * 1000000 + time.tv_usec;
}

int NDL_PollEvent(char *buf, int len)
{
  int fd = open("/dev/events", "r");
  return read(fd, buf, len);
}

void NDL_OpenCanvas(int *w, int *h)
{
  if (getenv("NWM_APP"))
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1)
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }
  int fd = open("/proc/dispinfo", "r");
  char buf[50];
  read(fd, buf, sizeof(buf));
  int width, height;
  sscanf(buf, "w : %d, h : %d", &width, &height);
  assert(width > 0);
  assert(height > 0);
  printf("screen_width : %d, screen_height : %d\n", width, height);
  if (*w == 0 && *h == 0)
  {
    *w = width;
    *h = height;
  }
  assert(*w <= width && *h <= height);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h)
{
  int fd = open("/proc/dispinfo", "r");
  char buf[50];
  read(fd, buf, sizeof(buf));
  int width, height;
  sscanf(buf, "w : %d, h : %d", &width, &height);

  fd = open("/dev/fb", "r");
  for (int r = 0; r < h; r++)
  {
    lseek(fd, x + (y + r) * width, SEEK_SET);
    write(fd, pixels + r * w, w);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples)
{
}

void NDL_CloseAudio()
{
}

int NDL_PlayAudio(void *buf, int len)
{
  return 0;
}

int NDL_QueryAudio()
{
  return 0;
}

int NDL_Init(uint32_t flags)
{
  if (getenv("NWM_APP"))
  {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit()
{
}
