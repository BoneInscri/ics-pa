
#include <klibtest.h>

char one_cat[50];
char two_cat[50];
void strcat_test()
{
    // glibc
    strcpy(one_cat, "ijk");
    check(strcat(one_cat, "lmn") == one_cat); /* Returned value. */
    str_equal(one_cat, "ijklmn");             /* Basic test. */

    strcpy(one_cat, "x");
    strcat(one_cat, "yz");
    str_equal(one_cat, "xyz");    /* Writeover. */
    str_equal(one_cat + 4, "mn"); /* Wrote too much? */

    strcpy(one_cat, "gh");
    strcpy(two_cat, "ef");
    strcat(one_cat, two_cat);
    str_equal(one_cat, "ghef");			/* Basic test encore. */
    str_equal(two_cat, "ef");			/* Stomped on source? */

    strcpy(one_cat, "");
    strcat(one_cat, "");
    str_equal(one_cat, "");			/* Boundary conditions. */
    strcpy(one_cat, "ab");
    strcat(one_cat, "");
    str_equal(one_cat, "ab");
    strcpy(one_cat, "");
    strcat(one_cat, "cd");
    str_equal(one_cat, "cd");
}