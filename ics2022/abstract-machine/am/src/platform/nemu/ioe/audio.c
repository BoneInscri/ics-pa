#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)
#define SBUF_SIZE 0x10000

static int w_idx = 0;

static void audio_write(uint8_t *buf, int len)
{
  int sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  assert(sbuf_size > 0);
  int nwrite = 0;
  int count = inl(AUDIO_COUNT_ADDR);
  while (nwrite < len)
  {
    if (count != SBUF_SIZE)
    {
      outb(AUDIO_SBUF_ADDR + w_idx % sbuf_size, *buf);
      w_idx++;
      nwrite++;
      buf++;
      count++;
    }
  }
  outl(AUDIO_COUNT_ADDR, count);
}

void __am_audio_init()
{
  outl(AUDIO_SBUF_SIZE_ADDR, SBUF_SIZE);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg)
{
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl)
{
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_COUNT_ADDR, 0);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat)
{
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl)
{
  int len = ctl->buf.end - ctl->buf.start;
  audio_write(ctl->buf.start, len);
}
