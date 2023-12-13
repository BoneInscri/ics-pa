#include <klibtest.h>

char one_mem[50];
char two_mem[50];
static void memset_test()
{
    strcpy(one_mem, "abcdefgh");
    check(memset(one_mem + 1, 'x', 3) == one_mem + 1); /* Return value. */
    str_equal(one_mem, "axxxefgh");                    /* Basic test. */

    memset(one_mem + 2, 'y', 0);
    str_equal(one_mem, "axxxefgh"); /* Zero-length set. */

    memset(one_mem + 5, 0, 1);
    str_equal(one_mem, "axxxe");  /* Zero fill. */
    str_equal(one_mem + 6, "gh"); /* And the leftover. */

    memset(one_mem + 2, 010045, 1);
    str_equal(one_mem, "ax\045xe"); /* Unsigned char convert. */
}

static void memcmp_test()
{
    check(memcmp("a", "a", 1) == 0);      /* Identity. */
    check(memcmp("abc", "abc", 3) == 0);  /* Multicharacter. */
    check(memcmp("abcd", "abce", 4) < 0); /* Honestly unequal. */
    check(memcmp("abce", "abcd", 4) > 0);
    check(memcmp("alph", "beta", 4) < 0);
    check(memcmp("a\203", "a\003", 2) > 0);
    check(memcmp("abce", "abcd", 3) == 0); /* Count limited. */
    check(memcmp("abc", "def", 0) == 0);   /* Zero count. */
}

static void memmove_test()
{
    check(memmove(one_mem, "abc", 4) == one_mem); /* Returned value. */
    str_equal(one_mem, "abc");                    /* Did the copy go right? */

    strcpy(one_mem, "abcdefgh");
    memmove(one_mem + 1, "xyz", 2);
    str_equal(one_mem, "axydefgh"); /* Basic test. */

    strcpy(one_mem, "abc");
    memmove(one_mem, "xyz", 0);
    str_equal(one_mem, "abc"); /* Zero-length copy. */

    strcpy(one_mem, "hi there");
    strcpy(two_mem, "foo");
    memmove(two_mem, one_mem, 9);
    str_equal(two_mem, "hi there"); /* Just paranoia. */
    str_equal(one_mem, "hi there"); /* Stomped on source? */

    strcpy(one_mem, "abcdefgh");
    memmove(one_mem + 1, one_mem, 9);
    str_equal(one_mem, "aabcdefgh"); /* Overlap, right-to-left. */

    strcpy(one_mem, "abcdefgh");
    memmove(one_mem + 1, one_mem + 2, 7);
    str_equal(one_mem, "acdefgh"); /* Overlap, left-to-right. */

    strcpy(one_mem, "abcdefgh");
    memmove(one_mem, one_mem, 9);
    str_equal(one_mem, "abcdefgh"); /* 100% overlap. */
}

void memcpy_test()
{
    check(memcpy(one_mem, "abc", 4) == one_mem); /* Returned value. */
    str_equal(one_mem, "abc");                   /* Did the copy go right? */

    strcpy(one_mem, "abcdefgh");
    memcpy(one_mem + 1, "xyz", 2);
    str_equal(one_mem, "axydefgh"); /* Basic test. */

    strcpy(one_mem, "abc");
    memcpy(one_mem, "xyz", 0);
    str_equal(one_mem, "abc"); /* Zero-length copy. */

    strcpy(one_mem, "hi there");
    strcpy(two_mem, "foo");
    memcpy(two_mem, one_mem, 9);
    str_equal(two_mem, "hi there"); /* Just paranoia. */
    str_equal(one_mem, "hi there"); /* Stomped on source? */
}

void mem_test()
{
    memset_test();
    memcmp_test();
    memmove_test();
    memcpy_test();
}