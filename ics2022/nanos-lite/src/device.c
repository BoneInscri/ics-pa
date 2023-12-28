#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len)
{
  for (int i = 0; i < len; i++)
  {
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len)
{
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  char message[70];
  memset(message, 0, sizeof(message));
  if (ev.keycode != AM_KEY_NONE)
  {
    assert(len <= 70);
    int ret = snprintf(message, len, "k%c %s", ev.keydown ? 'd' : 'u', keyname[ev.keycode]);
    strncpy(buf, message, ret);   
    assert(ret <= len);
    return ret;
  }
  else
  {
    return 0;
  }
}

size_t dispinfo_read(void *buf, size_t offset, size_t len)
{
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int width = cfg.width;
  int height = cfg.height;
  return sprintf((char*)buf, "w : %d, h : %d", width, height);
}

size_t fb_write(const void *buf, size_t offset, size_t len)
{
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int width = cfg.width;
  // int height = cfg.height;

  int x = offset % width;
  int y = offset / width;

  // printf("y : %d, x : %d, len : %d, width : %d\n", y, x, len, width);
  assert(x + len < width);
  io_write(AM_GPU_FBDRAW, x, y, (uint32_t*)buf, len, 1, true);

  return len;
}

void init_device()
{
  Log("Initializing devices...");
  ioe_init();
}
