#include <cxstring.h>

cxexception_t CXException_String = {"CXException_String", "string exception", CXERROR_ARG};
cxexception_t CXException_String_InvalidArg = {"CXException_String_InvalidArg", "invalid function argument", CXERROR_ARG};
cxexception_t CXException_String_InvalidPointer = {"CXException_String_InvalidPointer", "invalid string pointer", CXERROR_ARG};
cxexception_t CXException_String_InvalidIndex = {"CXException_String_InvalidIndex", "invalid string index", CXERROR_ARG};
cxexception_t CXException_String_InvalidSize = {"CXException_String_InvalidSize", "invalid string size", CXERROR_ARG};
cxexception_t CXException_String_InvalidCap = {"CXException_String_InvalidCap", "invalid string capacity", CXERROR_ARG};
cxexception_t CXException_String_MissingNull = {"CXException_String_MissingNull", "string is missing terminating null character", CXERROR_ARG};

cxstring_t cxstring_construct(cxmem_storage_t *storage, char *data, cxsize_t cap)
{
    cxstring_t string = {};
    cxsize_t size = 0;

    if(cap <= 0)
        cxthrow(CXException_String_InvalidArg);

    if(data)
        size = strlen(data);

    if(cap <= size + 1)
        cap = size * 2;

    if(cap < CXSTRING_DEFAULT_CAP)
        cap = CXSTRING_DEFAULT_CAP;

    string.data = cxmalloc(storage, cap);
    string.size = size;
    string.cap = cap;

    if(data)
        memcpy(string.data, data, size + 1);

    return string;
}

void cxstring_destruct(cxstring_t string)
{
    cxfree(string.data);

    return;
}

void cxstring_verify(cxstring_t *string)
{
    if(!string)
        cxthrow(CXException_String_InvalidArg);

    if(!string->data)
        cxthrow(CXException_String_InvalidPointer);

    if(string->cap <= 0)
        cxthrow(CXException_String_InvalidCap);

    if(string->size < 0 || string->size > string->cap)
        cxthrow(CXException_String_InvalidSize);

    if(string->index > string->size)
        cxthrow(CXException_String_InvalidIndex);

    return;
}

cxstring_t cxstring_cap(char *data, cxsize_t cap)
{
    return cxstring_construct(cxauto, data, cap);
}

cxstring_t cxstring(char *data)
{
    return cxstring_construct(cxauto, data, CXSTRING_DEFAULT_CAP);
}

cxstring_t cxstring_at(cxstring_t string, cxsize_t i)
{
    string.index = i;

    return string;
}

cxstring_t cxstring_sub(cxstring_t string, cxsize_t i, cxsize_t size)
{
    string.index = i;
    string.size = i + size;

    return string;
}

cxsize_t cxstring_size(cxstring_t string)
{
    cxsize_t i = string.index;

    cxstring_verify(&string);

    for(; string.data[i]; i++)
        if(i == string.cap - 1)
            cxthrow(CXException_String_MissingNull);

    return i;
}

cxstring_t cxstring_resize(cxstring_t *string, cxsize_t size)
{
    cxstring_reserve(string, size + 1);

    if(size > string->size)
        memset(&string->data[string->size], ' ', size - string->size);

    string->size = size;
    string->data[string->size] = 0;

    return *string;
}

cxstring_t cxstring_reserve(cxstring_t *string, cxsize_t size)
{
    cxstring_verify(string);

    if(size + 1 >= string->cap)
    {
        string->data = cxrealloc(string->data, 2 * size);
        string->cap = 2 * size;
    }

    return *string;
}

cxstring_t cxstring_clear(cxstring_t *string)
{
    cxstring_verify(string);

    string->size = 0;
    string->data[0] = 0;

    return *string;
}

int cxstring_empty(cxstring_t string)
{
    cxstring_verify(&string);

    return !string.size;
}

cxstring_t cxstring_erase(cxstring_t string, cxsize_t size)
{
    cxstring_verify(&string);

    if(!size)
        return string;

    if(size < 0 || (string.size - string.index) < size)
        cxthrow(CXException_String_InvalidArg);

    memmove(&string.data[string.index], &string.data[string.index + size], (string.size - string.index) - size);

    string.size -= size;
    string.data[string.size] = 0;
    string.index = 0;

    return string;
}

cxstring_t cxstring_replace(cxstring_t des, cxsize_t size, cxstring_t src)
{
    cxsize_t new_des_size = 0;

    cxstring_verify(&des);
    cxstring_verify(&src);

    if(size > des.size - des.index)
        size = des.size - des.index;

    new_des_size = des.size - size + src.size - src.index;

    cxstring_reserve(&des, new_des_size);

    memmove(&des.data[des.index + src.size],  &des.data[des.index + size], des.size - des.index - size);
    memcpy(&des.data[des.index], &src.data[src.index], src.size - src.index);

    des.size = new_des_size;
    des.data[des.size] = 0;
    des.index = 0;

    return des;
}

cxstring_t cxstring_insert(cxstring_t des, cxsize_t index, cxstring_t src)
{
    return cxstring_replace(cxstring_at(des, index), 0, src);
}

cxstring_t cxstring_copy(cxstring_t des, cxstring_t src)
{
    return cxstring_replace(des, src.size - src.index, src);
}

cxstring_t cxstring_concat(cxstring_t des, cxstring_t src)
{
    return cxstring_insert(des, des.size, src);
}

cxstring_t cxstring_swap(cxstring_t *str1, cxstring_t *str2)
{
    cxstring_t *str_min = NULL, *str_max = NULL;
    cxsize_t i = 0, j = 0, tmp2 = 0;
    char tmp = 0;

    cxstring_verify(str1);
    cxstring_verify(str2);

    if(str1->size < str2->size)
    {
        str_min = str1;
        str_max = str2;
    }
    else
    {
        str_min = str2;
        str_max = str1;
    }

    if(str_min->cap <= str_max->size + 1)
    {
        str_min->data = cxrealloc(str_min->data, str_max->size * 2);
        str_min->cap = str_max->size * 2;
    }

    i = str_min->index;
    j = str_max->index;

    for(; i < str_min->size; i++, j++)
    {
        tmp = str1->data[i];
        str1->data[i] = str2->data[j];
        str2->data[j] = tmp;
    }

    for(; j < str_max->size; i++, j++)
        str_min->data[i] = str_max->data[j];

    tmp2 = str_min->size;
    str_min->size = str_max->size;
    str_max->size = tmp2;

    str_min->data[str_min->size] = 0;
    str_max->data[str_max->size] = 0;

    return *str1;
}

cxstring_t cxstring_clone(cxstring_t str)
{
    return cxstring(str.data);
}

int cxstring_scanf(cxstring_t string, char *format, ...)
{
    va_list argv;
    va_start(argv, format);

    return cxstring_vscanf(string, format, argv);
}

int cxstring_vscanf(cxstring_t string, char *format, va_list argv)
{
    int ret = 0;

    cxstring_size(string);

    ret = vsscanf(string.data, format, argv);

    if(errno)
        cxthrow(CXException_String_InvalidArg);

    return ret;
}

cxstring_t cxstring_printf(cxstring_t string, char *format, ...)
{
    va_list argv;
    va_start(argv, format);

    return cxstring_vprintf(string, format, argv);
}

cxstring_t cxstring_vprintf(cxstring_t string, char *format, va_list argv)
{
    cxsize_t new_size = 0;

    new_size = vsnprintf(string.data, string.cap, format, argv);

    if(new_size > string.cap)
    {
        cxstring_reserve(&string, new_size);

        vsnprintf(string.data, string.cap, format, argv);
    }

    if(errno)
        cxthrow(CXException_String_InvalidArg);

    return string;
}
