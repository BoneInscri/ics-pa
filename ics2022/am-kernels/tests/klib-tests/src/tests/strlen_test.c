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
}
