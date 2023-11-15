#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

char *strcpy(char *dst, const char *src)
{
  int dst_p = 0;
  int src_p = 0;
  while (src[src_p])
  {
    dst[dst_p++] = src[src_p++];
  }
  dst[dst_p] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  panic("Not implemented");
}

char *strcat(char *dst, const char *src)
{
  int dst_p = 0;
  int src_p = 0;

  while (dst[dst_p])
    dst_p++;
  while (src[src_p])
  {
    dst[dst_p++] = src[src_p++];
  }
  dst[dst_p] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 && *s2 && *s1 == *s2)
  {
    s1++;
    s2++;
  }
  int ret = *s1 - *s2;
  if (ret == 0)
  {
    return 0;
  }
  else if (ret > 0)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n)
{
  for (int i = 0; i < n; i++)
  {
    *((int *)s + i) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n)
{
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  for (int i = 0; i < n; i++)
  {
    if (*((uint8_t *)s1 + i) < *((uint8_t *)s2 + i))
    {
      return -1;
    }
    else if (*((uint8_t *)s1 + i) > *((uint8_t *)s2 + i))
    {
      return 1;
    }
  }
  return 0;
  // panic("Not implemented");
}
#endif
