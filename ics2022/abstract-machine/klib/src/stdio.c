#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...)
{
  panic("Not implemented");
}

static int int2str(int num, char *num_str)
{
  int ret = 0;
  if (num < 0)
  {
    // 负数
    *num_str = '-';
    num_str++;
    num = -num;
    ret++;
  } else if (num == 0) {
    // 0 
    *num_str = '0';
    num_str++;
    ret++;
  }

  int len_num = 0;
  char num_tmp[20];
  while (num)
  {
    num_tmp[len_num++] = num % 10 + '0';
    num /= 10;
  }
  for (int i = len_num - 1; i >= 0; i--)
  {
    *num_str = num_tmp[i];
    num_str++;
  }
  *num_str = '\0';
  return ret + len_num;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  int d;
  // char c;
  char *s;
  int s_len = 0;
  int out_idx = 0;
  char buf[30];

  while (*fmt)
  {
    if (*fmt == '%')
    {
      fmt++;
      switch (*fmt)
      {
      case 's': /* string */
        s = va_arg(ap, char *);
        s_len = strlen(s);
        strcpy(out + out_idx, s);
        break;
      case 'd': /* int */
        d = va_arg(ap, int);
        s_len = int2str(d, buf);
        strcpy(out + out_idx, buf);
        out_idx += s_len;
        break;
        // case 'c':              /* char */
        //     /* need a cast here since va_arg only
        //       takes fully promoted types */
        //     c = (char) va_arg(ap, int);
        //     break;
      }
    }
    else
    {
      out[out_idx++] = *fmt;
    }
    fmt++;
  }
  out[out_idx] = '\0';
  return out_idx;
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list ap;
  
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);

  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  int d;
  char *s;
  int s_len = 0;
  int out_idx = 0;
  char buf[30];

  while (*fmt && n)
  {
    if (*fmt == '%')
    {
      fmt++;
      switch (*fmt)
      {
      case 's': /* string */
        s = va_arg(ap, char *);
        s_len = strlen(s);
        strcpy(out + out_idx, s);

        out_idx += MIN(s_len, n);
        n -= out_idx;
        break;
      case 'd': /* int */
        d = va_arg(ap, int);
        s_len = int2str(d, buf);
        strcpy(out + out_idx, buf);

        out_idx += MIN(s_len, n);// !!!
        n -= out_idx;// !!!
        break;
        // case 'c':              /* char */
        //     /* need a cast here since va_arg only
        //       takes fully promoted types */
        //     c = (char) va_arg(ap, int);
        //     break;
      }
    }
    else
    {
      out[out_idx++] = *fmt;
      n--;
    }
    fmt++;
  }
  out[out_idx] = '\0';
  return out_idx;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);

  return ret;
}


#endif
