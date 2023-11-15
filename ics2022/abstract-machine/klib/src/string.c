#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  // panic("Not implemented");
  size_t len = 0;
  while (s[len])
  {
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src)
{
  panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n)
{
  panic("Not implemented");
}

char *strcat(char *dst, const char *src)
{
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2)
{
  size_t s1_len = strlen(s1);
  size_t s2_len = strlen(s2);
  if (s1_len < s2_len)
    return -1;
  else if (s1_len > s2_len)
    return 1;
  while (s1 && s2)
  {
    if (*s1 < *s2)
    {
      return -1;
    }
    else if (*s1 > *s2)
    {
      return 1;
    }
    s1++;
    s2++;
  }
  return 0;
  // panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n)
{
  panic("Not implemented");
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
  panic("Not implemented");
}

#endif
