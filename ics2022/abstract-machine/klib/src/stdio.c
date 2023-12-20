#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2     /* unsigned/signed long */
#define PLUS 4     /* show plus */
#define SPACE 8    /* space if plus */
#define LEFT 16    /* left justified */
#define SPECIAL 32 /* 0x */
#define LARGE 64   /* use 'ABCDEF' instead of 'abcdef' */
#define do_div(n, base) ({ \
    int __res; \
    __res = ((unsigned long) n) % (unsigned) base; \
    n = ((unsigned long) n) / (unsigned) base; \
    __res; })
#define vsnprintf_writechar(size, write_cnt, str, c) \
  do                                                 \
  {                                                  \
    *str++ = (c);                                    \
    if (++write_cnt >= size - 1)                     \
    {                                                \
      goto finish;                                   \
    }                                                \
  } while (0)

static int __atoi(const char **s)
{
  int i = 0;

  while (is_digit(**s))
    i = i * 10 + *((*s)++) - '0';
  return i;
}

static void __number(long num, int base, int size, int precision, int type, int* cnt)
{
  char c, sign, tmp[66];
  const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i;

  if (type & LARGE)
    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 36)
    return;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (type & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (type & SPACE)
    {
      sign = ' ';
      size--;
    }
  }
  if (type & SPECIAL)
  {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }
  i = 0;
  if (num == 0) {
    tmp[i++] = '0';
  }
  else {
    while (num != 0) 
      tmp[i++] = digits[do_div(num, base)];
  }
  if (i > precision) {
    precision = i;
  }
  size -= precision;
  if (!(type & (ZEROPAD + LEFT))) {
    while (size-- > 0) {
      putch(' ');
      (*cnt)++;
    }
  }
  if (sign) {
    putch(sign);
    (*cnt)++;
  }
  if (type & SPECIAL)
  {
    if (base == 8) {
      putch('0');
      (*cnt)++;
    }
    else if (base == 16)
    {
      putch('0');
      putch(digits[33]);
      (*cnt) +=2;
    }
  }
  if (!(type & LEFT))
  {
    while (size-- > 0) {
      putch(c);
      (*cnt)++;
    }
  }
  while (i < precision--) {
    putch('0');
    (*cnt)++;
  }
  while (i-- > 0) {
    putch(tmp[i]);
    (*cnt)++;
  }
  while (size-- > 0) {
    putch(' ');
    (*cnt)++;
  }
}

static char *__number_buf(char *buf, long num, int base, int size, int precision, int type)
{
  char c, sign, tmp[66];
  const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i;

  if (type & LARGE)
    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 36)
    return 0;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (type & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (type & SPACE)
    {
      sign = ' ';
      size--;
    }
  }
  if (type & SPECIAL)
  {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }
  i = 0;
  if (num == 0)
    tmp[i++] = '0';
  else
    while (num != 0)
      tmp[i++] = digits[do_div(num, base)];
  if (i > precision)
    precision = i;
  size -= precision;
  if (!(type & (ZEROPAD + LEFT)))
    while (size-- > 0)
      *buf++ = ' ';
  if (sign)
    *buf++ = sign;
  if (type & SPECIAL)
  {
    if (base == 8)
      *buf++ = '0';
    else if (base == 16)
    {
      *buf++ = '0';
      *buf++ = digits[33];
    }
  }
  if (!(type & LEFT))
  {
    while (size-- > 0)
      *buf++ = c;
  }
  while (i < precision--)
    *buf++ = '0';
  while (i-- > 0)
    *buf++ = tmp[i];
  while (size-- > 0)
    *buf++ = ' ';
  return buf;
}

int vprintf(const char *fmt, va_list ap)
{
  int flags;       /* flags to number() */
  int field_width; /* width of output field */
  int precision;   /* min. # of digits for integers; max number of chars for from string */
  int qualifier;   /* 'l', or 'L' for integer fields */

  int base;
  int len;
  unsigned long num;
  char *s;

  int cnt = 0;

  for (; *fmt; ++fmt)
  {
    if (*fmt != '%')
    {
      putch(*fmt);
      cnt++;
      continue;
    }
    flags = 0;
  repeat:
    ++fmt; /* this also skips first '%' */
    switch (*fmt)
    {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }
    /* get field width */
    field_width = -1;
    if (is_digit(*fmt))
    {
      field_width = __atoi(&fmt);
    }
    else if (*fmt == '*')
    {
      ++fmt;
      /* it's the next argument */
      field_width = va_arg(ap, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.')
    {
      ++fmt;
      if (is_digit(*fmt))
        precision = __atoi(&fmt);
      else if (*fmt == '*')
      {
        ++fmt;
        /* it's the next argument */
        precision = va_arg(ap, int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      ++fmt;
    }

    /* default base */
    base = 10;
    switch (*fmt)
    {
    case 'c':
      if (!(flags & LEFT))
      {
        while (--field_width > 0)
        {
          putch(' ');
          cnt++;
        }
      }
      putch((unsigned char)va_arg(ap, int));
      cnt++;

      while (--field_width > 0)
      {
        putch(' ');
        cnt++;
      }
      continue;
    case 's':
      s = va_arg(ap, char *);
      if (!s)
        s = "<NULL>";
      // len = strnlen(s, precision);
      len = strlen(s);

      if (!(flags & LEFT)) {
        while (len < field_width--) {
          putch(' ');
          cnt++;
        }
      }
      for (int i = 0; i < len; ++i) {
        putch(*s++);
        cnt++;
      }
      while (len < field_width--) {
        putch(' ');
        cnt++;
      }
      continue;

    case 'p':
      if (field_width == -1)
      {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      __number((unsigned long)va_arg(ap, void *), 16, field_width, precision, flags, &cnt);
      continue;

    case 'o':
      base = 8;
      break;

    case 'X':
      flags |= LARGE;
    case 'x':
      base = 16;
      break;
    case 'd':
    case 'i':
      flags |= SIGN;
    case 'u':
      break;
    default:
      if (*fmt != '%') {
        putch('%');
        cnt++;
      }
      if (*fmt) {
        putch(*fmt);
        cnt++;
      }
      else
        --fmt;
      continue;
    }
    if (qualifier == 'l')
      num = va_arg(ap, unsigned long);
    else if (flags & SIGN)
      num = va_arg(ap, int);
    else
      num = va_arg(ap, unsigned int);
    __number(num, base, field_width, precision, flags, &cnt);
  }
  return cnt;
}

int vsprintf(char *str, const char *fmt, va_list ap)
{
  int flags;       /* flags to number() */
  int field_width; /* width of output field */
  int precision;   /* min. # of digits for integers; max number of chars for from string */
  int qualifier;   /* 'l', or 'L' for integer fields */

  char *buf = str;
  int base;
  int len;
  unsigned long num;
  char *s;

  for (; *fmt; ++fmt)
  {
    if (*fmt != '%')
    {
      *buf++ = *fmt;
      continue;
    }
    /* process flags */
    flags = 0;
  repeat:
    ++fmt; /* this also skips first '%' */
    switch (*fmt)
    {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }
    /* get field width */
    field_width = -1;
    if (is_digit(*fmt))
    {
      field_width = __atoi(&fmt);
    }
    else if (*fmt == '*')
    {
      ++fmt;
      /* it's the next argument */
      field_width = va_arg(ap, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }
    /* get the precision */
    precision = -1;
    if (*fmt == '.')
    {
      ++fmt;
      if (is_digit(*fmt))
        precision = __atoi(&fmt);
      else if (*fmt == '*')
      {
        ++fmt;
        /* it's the next argument */
        precision = va_arg(ap, int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      ++fmt;
    }

    /* default base */
    base = 10;
    switch (*fmt)
    {
    case 'c':
      if (!(flags & LEFT))
        while (--field_width > 0)
          *buf++ = ' ';
      *buf++ = (unsigned char)va_arg(ap, int);
      while (--field_width > 0)
        *buf++ = ' ';
      continue;

    case 's':
      s = va_arg(ap, char *);
      if (!s)
        s = "<NULL>";

      // len = strnlen(s, precision);
      len = strlen(s);

      if (!(flags & LEFT))
        while (len < field_width--)
          *buf++ = ' ';
      for (int i = 0; i < len; ++i)
        *buf++ = *s++;
      while (len < field_width--)
        *buf++ = ' ';
      continue;

    case 'p':
      if (field_width == -1)
      {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      buf = __number_buf(buf, (unsigned long)va_arg(ap, void *), 16, field_width, precision, flags);
      continue;

    case 'n':
      if (qualifier == 'l')
      {
        long *ip = va_arg(ap, long *);
        *ip = (buf - str);
      }
      else
      {
        int *ip = va_arg(ap, int *);
        *ip = (buf - str);
      }
      continue;

    /* integer number formats - set up the flags and "break" */
    case 'o':
      base = 8;
      break;

    case 'X':
      flags |= LARGE;
    case 'x':
      base = 16;
      break;

    case 'd':
    case 'i':
      flags |= SIGN;
    case 'u':
      break;

    default:
      if (*fmt != '%')
        *buf++ = '%';
      if (*fmt)
        *buf++ = *fmt;
      else
        --fmt;
      continue;
    }
    if (qualifier == 'l')
      num = va_arg(ap, unsigned long);
    else if (flags & SIGN)
      num = va_arg(ap, int);
    else
      num = va_arg(ap, unsigned int);
    buf = __number_buf(buf, num, base, field_width, precision, flags);
  }
  *buf = '\0';
  return buf - str;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
  int flags;       /* flags to number() */
  int field_width; /* width of output field */
  int precision;   /* min. # of digits for integers; max number of chars for from string */
  int qualifier;   /* 'l', or 'L' for integer fields */

  char *buf;
  int base;
  int len;
  unsigned long num;
  char *s;

  int write_cnt = 0;

  if (size == 0)
  {
    return 0;
  }

  for (buf = str; *fmt; ++fmt)
  {
    if (*fmt != '%')
    {
      vsnprintf_writechar(size, write_cnt, buf, *fmt);
      continue;
    }

    /* process flags */
    flags = 0;
  repeat:
    ++fmt; /* this also skips first '%' */
    switch (*fmt)
    {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }

    /* get field width */
    field_width = -1;
    if (is_digit(*fmt))
    {
      field_width = __atoi(&fmt);
    }
    else if (*fmt == '*')
    {
      ++fmt;
      /* it's the next argument */
      field_width = va_arg(ap, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.')
    {
      ++fmt;
      if (is_digit(*fmt))
        precision = __atoi(&fmt);
      else if (*fmt == '*')
      {
        ++fmt;
        /* it's the next argument */
        precision = va_arg(ap, int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      ++fmt;
    }

    /* default base */
    base = 10;

    switch (*fmt)
    {
    case 'c':
      if (!(flags & LEFT))
        while (--field_width > 0)
          vsnprintf_writechar(size, write_cnt, buf, ' ');
      vsnprintf_writechar(size, write_cnt, buf, (unsigned char)va_arg(ap, int));
      while (--field_width > 0)
        vsnprintf_writechar(size, write_cnt, buf, ' ');
      continue;

    case 's':
      s = va_arg(ap, char *);
      if (!s)
        s = "<NULL>";

      // len = strnlen(s, precision);
      len = strlen(s);

      if (!(flags & LEFT))
        while (len < field_width--)
          vsnprintf_writechar(size, write_cnt, buf, ' ');
      for (int i = 0; i < len; i++)
        vsnprintf_writechar(size, write_cnt, buf, *s++);
      while (len < field_width--)
        vsnprintf_writechar(size, write_cnt, buf, ' ');
      continue;

    case 'p':
      if (field_width == -1)
      {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      buf = __number_buf(buf, (unsigned long)va_arg(ap, void *), 16, field_width, precision, flags);

      write_cnt = (field_width >= (size - 1 - write_cnt)) ? 0 : write_cnt - field_width;
      if (write_cnt == 0)
      {
        goto finish;
      }
      continue;

    case 'n':
      if (qualifier == 'l')
      {
        long *ip = va_arg(ap, long *);
        *ip = (buf - str);
      }
      else
      {
        int *ip = va_arg(ap, int *);
        *ip = (buf - str);
      }
      continue;

    /* integer number formats - set up the flags and "break" */
    case 'o':
      base = 8;
      break;

    case 'X':
      flags |= LARGE;
    case 'x':
      base = 16;
      break;

    case 'd':
    case 'i':
      flags |= SIGN;
    case 'u':
      break;

    default:
      if (*fmt != '%')
        vsnprintf_writechar(size, write_cnt, buf, ' ');
      if (*fmt)
        vsnprintf_writechar(size, write_cnt, buf, *fmt);
      else
        --fmt;
      continue;
    }
    if (qualifier == 'l')
      num = va_arg(ap, unsigned long);
    else if (flags & SIGN)
      num = va_arg(ap, int);
    else
      num = va_arg(ap, unsigned int);
    buf = __number_buf(buf, num, base, field_width, precision, flags);
  }

finish:
  *buf = '\0';
  return ((size > (buf - str)) ? (buf - str) : size);
}

int printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  int ret = vprintf(fmt, ap);
  va_end(ap);

  return ret;
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);

  return ret;
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
