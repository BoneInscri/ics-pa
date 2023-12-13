
#include <klibtest.h>

char one[50];
char two[50];
void strcpy_test()
{
    // from glibc
    check(strcpy(one, "abcd") == one); /* Returned value. */
    str_equal(one, "abcd");            /* Basic test. */

    strcpy(one, "x");
    str_equal(one, "x");      /* Writeover. */
    str_equal(one + 2, "cd"); /* Wrote too much? */

    strcpy(two, "hi there");
    strcpy(one, two);
    str_equal(one, "hi there"); /* Basic test encore. */
    str_equal(two, "hi there"); /* Stomped on source? */

    strcpy(one, "");
    str_equal(one, ""); /* Boundary condition. */

    // from glibc
    check(strncpy(one, "abc", 4) == one); /* Returned value. */
    str_equal(one, "abc");                /* Did the copy go right? */

    strcpy(one, "abcdefgh");
    strncpy(one, "xyz", 2);
    str_equal(one, "xycdefgh"); /* Copy cut by count. */

    strcpy(one, "abcdefgh");
    strncpy(one, "xyz", 3); /* Copy cut just before NUL. */
    str_equal(one, "xyzdefgh");

    strcpy(one, "abcdefgh");
    strncpy(one, "xyz", 4); /* Copy just includes NUL. */
    str_equal(one, "xyz");
    str_equal(one + 4, "efgh"); /* Wrote too much? */

    strcpy(one, "abcdefgh");
    strncpy(one, "xyz", 5); /* Copy includes padding. */
    str_equal(one, "xyz");
    str_equal(one + 4, "");
    str_equal(one + 5, "fgh");

    strcpy(one, "abc");
    strncpy(one, "xyz", 0); /* Zero-length copy. */
    str_equal(one, "abc");

    strncpy(one, "", 2); /* Zero-length source. */
    str_equal(one, "");
    str_equal(one + 1, "");
    str_equal(one + 2, "c");

    strcpy(one, "hi there");
    strncpy(two, one, 9);
    str_equal(two, "hi there"); /* Just paranoia. */
    str_equal(one, "hi there"); /* Stomped on source? */
}