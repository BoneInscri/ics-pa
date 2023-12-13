#include <klibtest.h>

char *str_list[] = {"abcdefg", "hhhhhiijlafjal", "hello world", "\n\nhello", "00000", "1303340"};
int ans[] = {7, 14, 11, 7, 5, 7};

#define NR_DATA LENGTH(str_list)

void strlen_test()
{
    for (int i = 0; i < NR_DATA; i++)
    {
        check(strlen(str_list[i]) == ans[i]);
    }

    // from glibc
    check(strlen("") == 0);     /* Empty. */
    check(strlen("a") == 1);    /* Single char. */
    check(strlen("abcd") == 4); /* Multiple chars. */
}
