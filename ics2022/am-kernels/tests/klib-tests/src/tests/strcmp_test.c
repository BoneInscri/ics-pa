#include <klibtest.h>

void strcmp_test()
{
    // taken from glibc
    check(strcmp("", "") == 0);       /* Trivial case. */
    check(strcmp("a", "a") == 0);     /* Identity. */
    check(strcmp("abc", "abc") == 0); /* Multicharacter. */
    check(strcmp("abc", "abcd") < 0); /* Length mismatches. */
    check(strcmp("abcd", "abc") > 0);
    check(strcmp("abcd", "abce") < 0); /* Honest miscompares. */
    check(strcmp("abce", "abcd") > 0);
    check(strcmp("a\203", "a") > 0); /* Tricky if char signed. */
    check(strcmp("a\203", "a\003") > 0);

    // taken from glibc
    check(strncmp("", "", 99) == 0);       /* Trivial case. */
    check(strncmp("a", "a", 99) == 0);     /* Identity. */
    check(strncmp("abc", "abc", 99) == 0); /* Multicharacter. */
    check(strncmp("abc", "abcd", 99) < 0); /* Length unequal. */
    check(strncmp("abcd", "abc", 99) > 0);
    check(strncmp("abcd", "abce", 99) < 0); /* Honestly unequal. */
    check(strncmp("abce", "abcd", 99) > 0);
    check(strncmp("a\203", "a", 2) > 0); /* Tricky if '\203' < 0 */
    check(strncmp("a\203", "a\003", 2) > 0);
    check(strncmp("abce", "abcd", 3) == 0); /* Count limited. */
    check(strncmp("abce", "abc", 3) == 0);   /* Count == length. */
    check(strncmp("abcd", "abce", 4) < 0);   /* Nudging limit. */
    check(strncmp("abc", "def", 0) == 0);    /* Zero count. */
}
