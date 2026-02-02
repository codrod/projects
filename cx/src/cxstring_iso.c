#include <cxstring.h>

int cxstring_strncasecmp(const char *str1, const char *str2, cxsize_t n)
{
    cxsize_t i = 0;

    for(; tolower(str1[i]) == tolower(str2[i]); i++)
        if(i + 1 == n)
            return 0;

    return str1[i] - str2[i];
}

int cxstring_compare(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    cxsize_t min_size = 0, size1 = 0, size2 = 0;
    int ret = 0;

    //validate strings
    cxstring_size(string1);
    cxstring_size(string2);

    size1 = string1.size - string1.index;
    size2 = string2.size - string2.index;

    if(!size1)
    {
        if(!size2)
            return 0;
        else
            return -1;
    }
    else if(!size2)
        return 1;

    if(size1 <= size2)
        min_size = size1;
    else
        min_size = size2;

    if(case_sensitive)
    {
        ret = strncmp(&string1.data[string1.index], &string2.data[string2.index], min_size);
        //printf("strncmp: '%s' '%s' %i\n", &string1.data[string1.index], &string2.data[string2.index], ret);
    }
    else
        ret = cxstring_strncasecmp(&string1.data[string1.index], &string2.data[string2.index], min_size);

    return ret;
}

int cxstring_equals(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    return !cxstring_compare(string1, string2, case_sensitive);
}

int cxstring_starts_with(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    if(string1.size < string2.size)
        return 0;

    return !cxstring_compare(cxstring_sub(string1, 0, string2.size), string2, case_sensitive);
}

int cxstring_ends_with(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    if(string1.size < string2.size)
        return 0;

    return !cxstring_compare(cxstring_sub(string1, string1.size - string2.size, string2.size), string2, case_sensitive);
}

int cxstring_contains(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    return cxstring_find(string1, string2, case_sensitive) != 0;
}

cxsize_t cxstring_find(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    cxsize_t i = string1.index;

    cxstring_verify(&string1);

    for(; i < string1.size; i++)
        if(cxstring_equals(cxstring_at(string1, i), string2, case_sensitive))
            return i + 1;

    return 0;
}

cxsize_t cxstring_span(cxstring_t string1, cxstring_t string2, int case_sensitive)
{
    cxsize_t i = string1.index, j = string2.index, span = 0;

    cxstring_verify(&string1);
    cxstring_verify(&string2);

    for(; i < string1.size && j < string2.size; i++, j++)
    {
        if(tolower(string1.data[i]) == tolower(string2.data[j]))
            span++;
        else
            break;
    }

    return span;
}

cxstring_t cxstring_trim(cxstring_t string)
{
    cxsize_t i = string.index;

    cxstring_verify(&string);

    while(i < string.size && isspace(string.data[i]))
        string = cxstring_erase(string, 1);

    for(i = string.size; i >= 0 && isspace(string.data[i]); i--)
        string = cxstring_erase(cxstring_at(string, i), 1);

    string.index = 0;

    return string;
}

cxstring_t cxstring_strip(cxstring_t string)
{
    cxsize_t i = string.index;

    cxstring_verify(&string);

    for(; i < string.size; i++)
    {
        if(isspace(string.data[i]))
        {
            string = cxstring_erase(cxstring_at(string, i), 1);
            i--;
        }
    }

    string.index = 0;

    return string;
}

cxstring_t cxstring_to_lower(cxstring_t string)
{
    cxsize_t i = 0;

    cxstring_verify(&string);

    for(; i < string.size; i++)
        string.data[i] = tolower(string.data[i]);

    return string;
}

cxstring_t cxstring_to_upper(cxstring_t string)
{
    cxsize_t i = 0;

    cxstring_verify(&string);

    for(; i < string.size; i++)
        string.data[i] = toupper(string.data[i]);

    return string;
}
