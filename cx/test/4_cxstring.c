#include <cxstring.h>

int main()
{
    __cxmain__

    printf("\nCXSTRING: Test started\n");

    cxstring_t str = cxstring_construct(NULL, "", 256);
    cxstring_t str2 = cxstring("12345");
    cxstring_t str3 = cxstring_cap("abcde", 256);
    cxstring_t str4 = cxstring("XYZ");
    cxsize_t size = 0, index = 0, ret = 0;

    cxstring_destruct(str);

    cxstring_t at = cxstring_at(str2, 1);

    if((index = at.index) != 1)
        printf("cxstring_at: wrong index %lli\n", index);

    cxstring_t sub = cxstring_sub(str2, 1, 3);

    if(sub.index != 1 || sub.size != 4)
        printf("cxstring_sub: wrong index or size %lli %lli\n", sub.index, sub.size);

    if((size = cxstring_size(str2)) != 5)
        printf("cxstring_size: wrong size %lli\n", size);

    cxstring_reserve(&str2, 256);

    if(str2.cap < 256)
        printf("cxstring_reserve: wrong cap %lli\n", str2.cap);

    cxstring_reserve(&str2, 3);

    if(str2.cap < 3 || str2.size != 5 || str2.size != cxstring_size(str2))
        printf("cxstring_reserve: wrong cap or size %lli %lli\n", str2.cap, str2.size);

    cxstring_resize(&str2, 3);

    if(str2.cap < 3 || str2.size != 3 || str2.size != cxstring_size(str2))
        printf("cxstring_resize: wrong cap or size %lli %lli\n", str2.cap, str2.size);

    cxstring_resize(&str2, 10);

    if(str2.cap < 10 || str2.size != 10 || str2.size != cxstring_size(str2) || str2.data[str2.size - 1] != ' ')
        printf("cxstring_resize: wrong cap or size %lli %lli\n", str2.cap, str2.size);

    cxstring_clear(&str2);

    if(str2.cap < 3 || str2.size != 0 || cxstring_size(str2))
        printf("cxstring_clear: wrong cap or size %lli %lli\n", str2.cap, str2.size);

    if(!cxstring_empty(str2))
        printf("cxstring_empty:\n");

    if(strcmp((str3 = cxstring_replace(str3, 3, cxstr("123"))).data, "123de"))
        printf("cxstring_replace: '%s'\n", str3.data);

    if(strcmp((str3 = cxstring_insert(str3, 3, cxstr("abc"))).data, "123abcde"))
        printf("cxstring_insert: '%s'\n", str3.data);

    if(strcmp((str3 = cxstring_copy(cxstring_at(str3, 8), cxstr("123"))).data, "123abcde123"))
        printf("cxstring_copy: '%s'\n", str3.data);

    if(strcmp((str3 = cxstring_concat(str3, cxstr("456"))).data, "123abcde123456"))
        printf("cxstring_concat: '%s'\n", str3.data);

    if(strcmp(cxstring_swap(&str3, &str4).data, "XYZ"))
        printf("cxstring_swap: '%s'\n", str3.data);

    if(strcmp(str4.data, "123abcde123456"))
        printf("cxstring_swap: '%s'\n", str4.data);

    if(!strcmp((str = cxstring_clone(str4)).data, "XYZ") || str.data == str4.data)
        printf("cxstring_clone:\n");

    int num = 0;

    if(cxstring_scanf(cxstr("12345"), "%i", &num) != 1 || num != 12345)
        printf("cxstring_scanf: %i\n", num);

    str2 = cxstring_printf(str2, "%i", num);

    if(strcmp("12345", str2.data))
        printf("cxstring_printf: %s\n", str2.data);

    printf("CXSTRING: Test finished\n");

    cxreturn 0;
}
