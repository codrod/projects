#include <cxstring.h>

int main()
{
    __cxmain__

    printf("\nCXSTRING ISO: Test started\n");

    cxstring_t str2 = cxstring("");
    cxstring_t str3 = cxstring_cap("abcde", 256);
    cxsize_t size = 0, index = 0, ret = 0;

    if(ret = cxstring_compare(str3, cxstr("abcde"), 1) || !cxstring_equals(str3, cxstr("abcde"), 1))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(!(ret = cxstring_compare(str3, cxstr("ABCDE"), 1)))
        printf("cxstring_compare: should not be equal %lli\n", ret);

    if(ret = cxstring_compare(str3, cxstr("abcde"), 0))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(ret = cxstring_compare(str3, cxstr("ABCDE"), 0))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if((ret = cxstring_compare(str3, cxstr(""), 0)) < 0)
        printf("cxstring_compare: should be more %lli\n", ret);

    if((ret = cxstring_compare(cxstr(""), str3, 0)) > 0)
        printf("cxstring_compare: should be less %lli\n", ret);

    if((ret = cxstring_compare(str3, cxstr(""), 1)) < 0)
        printf("cxstring_compare: should be more %lli\n", ret);

    if((ret = cxstring_compare(cxstr(""), str3, 1)) > 0)
        printf("cxstring_compare: should be less %lli\n", ret);

    if(ret = cxstring_compare(str2, cxstr(""), 0))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(ret = cxstring_compare(cxstr(""), str2, 0))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(ret = cxstring_compare(str2, cxstr(""), 1))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(ret = cxstring_compare(cxstr(""), str2, 1))
        printf("cxstring_compare: should be equal %lli\n", ret);

    if(!cxstring_starts_with(cxstr("abc"), cxstr("abc"), 1))
        printf("cxstring_starts_with: should start with\n");

    if(!cxstring_starts_with(cxstr("abcdefghij"), str3, 1))
        printf("cxstring_starts_with: should start with\n");

    if(!cxstring_starts_with(cxstr("abcdefghij"), cxstr(""), 1))
        printf("cxstring_starts_with: should start with\n");

    if(cxstring_starts_with(cxstr("abc"), str3, 1))
        printf("cxstring_starts_with: should not start with\n");

    if(cxstring_starts_with(str2, cxstr("abcdefghij"), 1))
        printf("cxstring_starts_with: should not start with\n");

    if(!cxstring_ends_with(cxstr("ahij"), cxstr("hij"), 1))
        printf("cxstring_ends_with: should end with\n");

    if(!cxstring_ends_with(cxstr("abcdefghij"), cxstr(""), 1))
        printf("cxstring_ends_with: should end with\n");

    if(cxstring_ends_with(cxstr("abc"), str3, 1))
        printf("cxstring_ends_with: should not end with\n");

    if(cxstring_ends_with(str2, cxstr("abcdefghij"), 1))
        printf("cxstring_ends_with: should not end with\n");

    if((ret = cxstring_find(cxstr("12345"), cxstr("123"), 1)) != 1)
        printf("cxstring_find: %lli\n", ret);

    if((ret = cxstring_find(cxstr("12345"), cxstr("5"), 1)) != 5)
        printf("cxstring_find: %lli\n", ret);

    if((ret = cxstring_find(cxstr("12345"), cxstr("45"), 1)) != 4)
        printf("cxstring_find: %lli\n", ret);

    if((ret = cxstring_find(cxstr("12345"), cxstr("3"), 1)) != 3)
        printf("cxstring_find: %lli\n", ret);

    if((ret = cxstring_find(cxstr("12345"), cxstr(""), 1)))
        printf("cxstring_find: should not find %lli\n", ret);

    if((ret = cxstring_find(cxstr("12345"), cxstr("a"), 1)))
        printf("cxstring_find: should not find %lli\n", ret);

    if((ret = cxstring_span(cxstr("12345"), cxstr("123"), 3)) != 3)
        printf("cxstring_span: wrong span %lli\n", ret);

    if((ret = cxstring_span(cxstr("12345"), cxstr("abc"), 3)))
        printf("cxstring_span: wrong span %lli\n", ret);

    if(!cxstring_equals(cxstring_strip(cxstr("   123  456    ")), cxstr("123456"), 1))
        printf("cxstring_strip: \n");

    if(!cxstring_equals(cxstring_trim(cxstr("   123  456    ")), cxstr("123  456"), 1))
        printf("cxstring_trim: \n");

    if(!cxstring_equals(cxstring_to_lower(cxstr("ABCDE")), cxstr("abcde"), 1))
        printf("cxstring_to_lower:\n");

    if(!cxstring_equals(cxstr("ABCDE"), cxstring_to_upper(cxstr("abcde")), 1))
        printf("cxstring_to_upper:\n");

    printf("CXSTRING ISO: Test finished\n");

    cxreturn 0;
}
