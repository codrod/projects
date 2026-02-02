/*! \file cxstring.h
	\brief Strings
*/
#if !CXSTRING_VERSION

	#define CXSTRING_VERSION 1000000L

	#include <string.h>
	#include <ctype.h>

	#include <cxrt.h>

	#define CXSTRING_DEFAULT_CAP 8

	#define cxstring_includes cxstring_contains
	#define cxstring_remove cxstring_erase
	#define cxstring_search cxstring_find
	#define cxstring_append cxstring_concat
	#define cxstring_is_empty cxstring_empty
	#define cxstring_substr cxstring_sub
	#define cxstring_substring cxstring_sub
	#define cxstr(data) cxstring(data)

	typedef struct cxstring
	{
		char *data;
		cxsize_t index;
		cxsize_t size;
		cxsize_t cap;
	}
	cxstring_t;

	cxstring_t cxstring_construct(cxmem_storage_t *storage, char *data, cxsize_t cap);
	void cxstring_destruct(cxstring_t str);
	void cxstring_verify(cxstring_t *str);

	//cxstring_cap("", 256);
	//cxstring_cap("abc", 256);
	//cxstring_cap(NULL, 256);
	//cxstring("abcdef");
	cxstring_t cxstring(char *data);
	cxstring_t cxstring_cap(char *data, cxsize_t cap);

	cxstring_t cxstring_at(cxstring_t str, cxsize_t i);
	cxstring_t cxstring_sub(cxstring_t str, cxsize_t i, cxsize_t size);
	cxstring_t cxstring_erase(cxstring_t str, cxsize_t size);

	cxsize_t cxstring_size(cxstring_t str);
	cxstring_t cxstring_resize(cxstring_t *str, cxsize_t size);
	cxstring_t cxstring_reserve(cxstring_t *str, cxsize_t cap);
	cxstring_t cxstring_clear(cxstring_t *str);

	cxstring_t cxstring_copy(cxstring_t des, cxstring_t src);
	cxstring_t cxstring_insert(cxstring_t des, cxsize_t i, cxstring_t src);
	cxstring_t cxstring_replace(cxstring_t des, cxsize_t size, cxstring_t src);
	cxstring_t cxstring_concat(cxstring_t des, cxstring_t src);
	cxstring_t cxstring_swap(cxstring_t *str1, cxstring_t *str2);
	cxstring_t cxstring_clone(cxstring_t str);

 	int cxstring_empty(cxstring_t str);
	int cxstring_compare(cxstring_t str1, cxstring_t str2, int case_sensitive);
	int cxstring_equals(cxstring_t str1, cxstring_t str2, int case_sensitive);
	int cxstring_starts_with(cxstring_t str1, cxstring_t str2, int case_sensitive);
	int cxstring_ends_with(cxstring_t str1, cxstring_t str2, int case_sensitive);
	int cxstring_contains(cxstring_t str1, cxstring_t str2, int case_sensitive);
	cxsize_t cxstring_find(cxstring_t str1, cxstring_t str2, int case_sensitive);
	cxsize_t cxstring_span(cxstring_t str1, cxstring_t str2, int case_sensitive);

	cxstring_t cxstring_trim(cxstring_t str);
	cxstring_t cxstring_strip(cxstring_t str);
	cxstring_t cxstring_to_lower(cxstring_t str);
	cxstring_t cxstring_to_upper(cxstring_t str);

	cxstring_t cxstring_vprintf(cxstring_t string, char *format, va_list argv);
 	int cxstring_vscanf(cxstring_t string, char *format, va_list argv);

	cxstring_t cxstring_printf(cxstring_t string, char *format, ...);
	int cxstring_scanf(cxstring_t string, char *format, ...);

	extern cxexception_t CXException_String;
	extern cxexception_t CXException_String_InvalidArg;
	extern cxexception_t CXException_String_InvalidPointer;
	extern cxexception_t CXException_String_InvalidIndex;
	extern cxexception_t CXException_String_InvalidSize;
	extern cxexception_t CXException_String_InvalidCap;
	extern cxexception_t CXException_String_MissingNull;

#endif
