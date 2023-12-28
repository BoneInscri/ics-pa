#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  assert(s);
  size_t len = 0;
  while (s[len] != '\0')
    len++;
  return len;
}


int strcmp(const char *s1, const char *s2)
{
  assert(s1);
  assert(s2);
  while (*s1 && *s2 && *s1 == *s2)
  {
    s1++;
    s2++;
  }
  int ret = (uint8_t)*s1 - (uint8_t)*s2;
  if (ret == 0)
    return 0;
  else if (ret > 0)
    return 1;
  else
    return -1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  assert(s1);
  assert(s2);
  if (!n)
  {
    return 0;
  }

  while (n > 0 && *s1 != '\0' && *s2 != '\0' && *s1 == *s2)
  {
    s1++;
    s2++;
    n--;
  }

  int ret = 0;
  if (!n)
  {
    ret = (uint8_t) * (s1 - 1) - (uint8_t) * (s2 - 1);
  }
  else
  {
    ret = (uint8_t)*s1 - (uint8_t)*s2;
  }

  if (ret == 0)
    return 0;
  else if (ret > 0)
    return 1;
  else
    return -1;
}

char *strcpy(char *dst, const char *src)
{
  assert(dst);
  assert(src);
  int src_len = strlen(src);
  if (dst >= src && dst < src + src_len)
  {
    int last_idx = src_len - 1;
    for (int i = last_idx; i >= 0; i--)
    {
      dst[i] = src[i];
    }
    dst[src_len] = '\0';
  }
  else
  {
    int dst_p = 0;
    int src_p = 0;
    while (src[src_p])
    {
      dst[dst_p++] = src[src_p++];
    }
    dst[dst_p] = '\0';
  }
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  assert(dst);
  assert(src);

  // pay attention to overlap
  int src_len = strlen(src);
  src_len = (n < src_len ? n : src_len); // restrict it!

  if (dst >= src && dst < src + src_len)
  {
    int last_idx = src_len - 1;
    for (int i = last_idx; i >= 0; i--)
    {
      dst[i] = src[i];
    }
    dst[src_len] = '\0';
  }
  else
  {
    // int dst_p = 0;
    // int src_p = 0;
    // while (src[src_p])
    // {
    //   dst[dst_p++] = src[src_p++];
    // }
    // dst[dst_p] = '\0';
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
      dst[i] = src[i];
    }
    while (i < n)
    {
      dst[i] = '\0';
      i++;
    }
  }
  dst[src_len] = '\0';
  // 使用\0进行补全
  return dst;
}

char *strcat(char *dst, const char *src)
{
  assert(dst);
  assert(src);
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

void *memset(void *s, int c, size_t n)
{
  assert(s);
  for (int i = 0; i < n; i++)
  {
    *((uint8_t *)s + i) = c;
  }
  return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  assert(s1);
  assert(s2);
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
}

// support overlap
void *memmove(void *dst, const void *src, size_t n)
{
  // printf("dst : %p, src : %p, n : %d\n", dst, src, n);
  const char *s;
  char *d;

  if (n == 0)
    return dst;

  s = src;
  d = dst;
  if (s < d && s + n > d)
  {
    s += n;
    d += n;
    while (n-- > 0)
    {
      *--d = *--s;
    }
  }
  else
    while (n-- > 0)
    {
      *d++ = *s++;
    }

  return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
  return memmove(dst, src, n);
}

#endif
