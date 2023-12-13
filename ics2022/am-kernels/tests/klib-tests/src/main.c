#include <klibtest.h>


void (*entry)() = NULL; // mp entry

__attribute__((noinline))
void check(bool cond) {
  if (!cond) halt(1);
}

void str_equal(char* a, char* b) {
  check(a != NULL && b != NULL && STREQ(a, b));
}

static const char *tests[256] = {
    ['1'] = "strlen_test",
    ['2'] = "strcmp_test",
    ['3'] = "strcpy_test"
    // ['2'] = "strcpy_test"
};

int main(const char *args)
{
  switch (args[0])
  {
    CASE('1', strlen_test);
    CASE('2', strcmp_test);
    CASE('3', strcpy_test);
    // CASE('2', strcpy_test);
  default:
    // printf("Usage: make run mainargs=*\n");
    for (int ch = 0; ch < 256; ch++)
    {
      if (tests[ch])
      {
        // printf("  %c: %s\n", ch, tests[ch]);
      }
    }
  }
  return 0;
}
