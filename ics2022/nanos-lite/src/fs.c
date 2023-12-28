#include <fs.h>
#include <device.h>

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct
{
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum
{
  FD_STDIN,
  FD_STDOUT,
  FD_STDERR,
  FD_DISPINFO,
  FD_FB,
  FD_EVENTS,
};

size_t invalid_read(void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
    [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define N_FILES ARRLEN(file_table)

void init_fs()
{
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int width = cfg.width;
  int height = cfg.height;
  file_table[FD_FB].size = width * height;
}

int fs_open(const char *pathname, int flags, int mode)
{
  for (int fd = 0; fd < N_FILES; fd++)
  {
    if (!strcmp(pathname, file_table[fd].name))
    {
      return fd;
    }
  }
  panic("can't find the file : %s\n", pathname);
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len)
{
  size_t offset_disk = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;
  size_t size = file_table[fd].size;
  if (fd != FD_EVENTS && fd != FD_FB && fd!=FD_DISPINFO)
  {
    if (open_offset > size)
    {
      panic("fd_read : over size");
    }
  }
  size_t ret = file_table[fd].read(buf, offset_disk + open_offset, len);
  file_table[fd].open_offset += len;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len)
{
  size_t offset_disk = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;
  size_t size = file_table[fd].size;

  if (fd != FD_STDIN && fd != FD_STDOUT && fd != FD_STDERR)
  {
    if (open_offset > size)
    {
      panic("fd_write : over size");
    }
  }
  size_t ret = file_table[fd].write(buf, offset_disk + open_offset, len);
  file_table[fd].open_offset += len;

  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  if (fd == FD_STDIN || fd == FD_STDIN || fd == FD_STDERR)
  {
    panic("fs_lseek : not tested");
  }

  switch (whence)
  {
  case SEEK_END:
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  case SEEK_SET:
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:
    file_table[fd].open_offset += offset;
    break;
  default:
    panic("fs_lseek : not tested");
    break;
  }

  return file_table[fd].open_offset;
}

int fs_close(int fd)
{
  return 0;
}
