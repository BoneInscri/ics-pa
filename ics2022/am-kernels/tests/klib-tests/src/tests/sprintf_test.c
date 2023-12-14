#include <klibtest.h>
#include <limits.h>

int data[] = {0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1,
              UINT_MAX / 17, INT_MAX / 17, UINT_MAX};
char* ans_1[] = {"0", "126322567", "2147483647", "-2147483648", "-2147483647", "252645135", "126322567", "-1"};

char* ans_2[] = {"0", "12632", "21474", "-2147", "-2147", "25264", "12632", "-1"};

#define NR_DATA LENGTH(data)

char buf[100];
void sprintf_test()
{
    // test for sprintf
    for (int i = 0; i < NR_DATA; i++)
    {
        check(sprintf(buf, "%d", data[i]) == strlen(ans_1[i]));
        str_equal(buf, ans_1[i]);
    }

    // test for snprintf
    for (int i = 0; i < 1; i++)
    {
        check(snprintf(buf, 5, "%d", data[i]) == strlen(ans_2[i]));
        str_equal(buf, ans_2[i]);
    }
}