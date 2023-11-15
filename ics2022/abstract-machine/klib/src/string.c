#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  size_t len = 0;
  while (s[len])   len++;
  return len;
}

char *strcpy(char *dst, const char *src)
{
  while(*src) {
    *dst = *src;
    src++;
  }
  *dst = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  panic("Not implemented");
}

char *strcat(char *dst, const char *src)
{
  while(*dst) dst++;
  while(*src) {
    *dst = *src;
    dst++;
    src++;
  }
  *dst = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2)
{
  while(*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  int ret = *s1-*s2;
  if(ret == 0) {
    return 0;
  } else if(ret > 0) {
    return 1;
  } else {
    return -1;
  }
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
