#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#include <wchar.h>
#include <string.h>
#include <ctype.h>

#include <float.h>
#include <limits.h>

#include <cxui.h>
#include <cxarray.h>

#if !CXUI_NO_CHAR_STR

/*
	char 'strto'
*/

	cxsize_t cxui_simple_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		const char *str = array.ptr;
		cxbyte_t *objb = obj;
		cxsize_t i = 0, j = 0;
		int pace = 1;
		CX_DEF_UNUSED(argv);

		memset(obj, 0, sizof);
	
		//This function will absorb leading whitespace as defined by isspace().
		while(isspace(str[i++]))
			if(i >= array.size)
				return errno = CX_ERR_EMP, 0;

		if(str[i] == '0' && (str[i + 1] == 'x' || str[i + 1] == 'X'))
		{
			//If "0x" is followed by at least one hexadecimal digit then return 0.
			if
			(
				!((str[i + 2] >= '0' && str[i + 2] <= '9') ||
				(str[i + 2] >= 'a' && str[i + 2] <= 'z') ||
				(str[i + 2] >= 'A' && str[i + 2] <= 'Z'))
			)
				return 0;
	
			i += 2;
		}

		for(; str[i]; i++)
		{
			//This function will not convert more than array.size characters.
			if(i >= array.size) return errno = CX_ERR_NUL, 0;
		
			//This function will not set more than sizof bytes in the object.
			if(j >= sizof) return errno = CX_ERR_OVE, 0;
	
			if(str[i] >= '0' && str[i] <= '9') objb[j] += str[i] - '0';
			else if(str[i] >= 'a' && str[i] <= 'z') objb[j] += (str[i] - 'a') + 0xa;
			else if(str[i] >= 'A' && str[i] <= 'Z') objb[j] += (str[i] - 'A') + 0xA;
			else break;
	
			//Every other nybble completes a byte and
			//increases the pointer by one. 
			if(pace == 2) pace = 1, j++;
			else pace++, objb[j] <<= 4;
		}

		return i;
	}

	cxsize_t cxui_unsigned_ints_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		unsigned long long int inte = 0;
		const char *str = array.ptr;
		int tmperrno = errno, nchars = 0;
		errno = 0;
		CX_DEF_UNUSED(argv);

		//This function will enforce bounds checking by
		//requiring the string to be null-terminated.
		if(!cxui_strlen(addr) && errno)
			return 0;
		else if(!sscanf(str, "%llu%n", &inte, &nchars) && !errno)
			return errno = CX_ERR_EMP, nchars;
		else if(errno)
			return 0;

		//This function will promote all integers to 'unsigned long long int'.
		if(sizof == sizeof(unsigned int))
		{
			if(inte > UINT_MAX)
				return errno = ERANGE, nchars;
	
			unsigned int tmp = (unsigned int)inte;
	
			memcpy(obj, &tmp, sizof);
		}
		else if(sizof == sizeof(unsigned long long int))
		{
			memcpy(obj, &inte, sizof);
		}
		else if(sizof == sizeof(unsigned long int))
		{
			if(inte > ULONG_MAX)
				return errno = ERANGE, nchars;

			unsigned long int tmp = (unsigned long int)inte;

			memcpy(obj, &tmp, sizof);
		}
		else if(sizof == sizeof(unsigned short int))
		{
			if(inte > USHRT_MAX)
				return errno = ERANGE, nchars;
		
			unsigned short int tmp = (unsigned short int)inte;
	
			memcpy(obj, &tmp, sizof);
		}
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_ints_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		long long int inte = 0;
		const char *str = array.ptr;
		int tmperrno = errno, nchars = 0;
		errno = 0;
		CX_DEF_UNUSED(argv);

		//This function will enforce bounds checking by
		//requiring the string to be null-terminated.
		if(!cxui_strlen(addr) && errno)
			return 0;
		else if(!sscanf(str, "%lli%n", &inte, &nchars) && !errno)
			return errno = CX_ERR_EMP, nchars;
		else if(errno)
			return 0;

		//This function will promote all integers to 'long long int'.
		if(sizof == sizeof(int))
		{
			if(inte > INT_MAX || inte < INT_MIN)
				return errno = ERANGE, nchars;
	
			int tmp = (int)inte;
	
			memcpy(obj, &tmp, sizof);
		}
		else if(sizof == sizeof(long long int))
		{
			memcpy(obj, &inte, sizof);
		}
		else if(sizof == sizeof(long int))
		{
			if(inte > LONG_MAX || inte < LONG_MIN)
				return errno = ERANGE, nchars;

			long int tmp = (long int)inte;

			memcpy(obj, &tmp, sizof);
		}
		else if(sizof == sizeof(short int))
		{
			if(inte > SHRT_MAX || inte < SHRT_MIN)
				return errno = ERANGE, nchars;
		
			short int tmp = (short int)inte;
	
			memcpy(obj, &tmp, sizof);
		}
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_floats_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		long double flt = 0;
		const char *str = array.ptr;
		int tmperrno = errno, nchars = 0;
		errno = 0;
		CX_DEF_UNUSED(argv);

		//This function will enforce bounds checking by
		//requiring the string to be null-terminated.
		if(!cxui_strlen(addr) && errno)
			return 0;
		else if(!sscanf(str, "%Le%n", &flt, &nchars) && !errno)
			return errno = CX_ERR_EMP, 0;
		else if(errno)
			return 0;

		//This function will promote all floating-points to 'long double'.
		if(sizof == sizeof(double))
		{
			if(flt > DBL_MAX || flt < -DBL_MAX)
				return errno = ERANGE, nchars;

			double tmp = (double)flt;

			memcpy(obj, &tmp, sizof);
		}
		else if(sizof == sizeof(long double))
		{
			memcpy(obj, &flt, sizof);
		}
		else if(sizof == sizeof(float))
		{
			if(flt > FLT_MAX || flt < -FLT_MAX)
				return errno = ERANGE, nchars;
		
			float tmp = (float)flt;
	
			memcpy(obj, &tmp, sizof);
		}
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_char_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(char) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		CX_DEF_UNUSED(argv);
	
		if(!array.size)
			return errno = CX_ERR_SIZ, 0;

		memcpy(obj, array.ptr, sizeof(char));

		return 1;
	}
	
	cxsize_t cxui_wchar_t_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(cxarray_t) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		wint_t wi = 0;
		wchar_t wc = 0;
		CX_DEF_UNUSED(argv);
		
		if(!array.size)
			return errno = CX_ERR_SIZ, 0;
		
		if((wi = btowc(*(char*)array.ptr)) == WEOF)
			return errno = CX_ERR_ARG, 0;
			
		wc = (wchar_t)wi;
		
		*(wchar_t*)obj = wc;
		
		return 1;
	}

	cxsize_t cxui_str_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(cxarray_t) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		cxsize_t len = 0;
		CX_DEF_UNUSED(argv);
	
		//A string to string conversion is just a string copy.
		if(!cxui_str_cpy(sizof, obj, addr) && errno)
			return 0;
	
		//This function will return the number of characters
		//copied to the destination string.
		if(array.len == -1)
		{
			if(!(len = cxui_strlen(addr)) && errno)
				return 0;
		}
		else len = array.len;
	
		return len;
	}

	cxsize_t cxui_time_t_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(time_t) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		struct tm date;
		time_t time;
		cxsize_t ret = 0;
	
		if(!(ret = cxui_struct_tm_strto(sizeof(struct tm), addr, &date, argv)) && errno)
			return 0;
	
		if((time = mktime(&date)) == (time_t)(-1))
			return errno = CX_ERR_ARG, 0;
		
		*(time_t*)obj = time;
	
		return ret;
	}

	cxsize_t cxui_struct_tm_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(struct tm) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		struct tm time;
		int tmperrno = errno, nchars = 0;
		cxsize_t i = 0;
		errno = 0;
		CX_DEF_UNUSED(argv);
		int yoff = 0;
	
		//This is a table representing the days in each month of the year.
		int mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
		//This function initializes the 'struct tm' object to 0.
		//So if the implementation's 'struct tm' has any nonstandard
		//members those members will be invalid unless they can have a 
		//default value of 0.
		memset(&time, 0, sizeof(struct tm));
	
		//This function ensures bounds-checking by requiring the string represented
		//by array to be null-terminated.
		if(!cxui_strlen(addr) && errno)
			return 0;
		else if(sscanf(array.ptr, "%i-%i-%iT%i:%i:%i%n", &time.tm_year, &time.tm_mon,
		   &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec, &nchars) != 6)
		{
			if(!errno) errno = CX_ERR_STR;
			return 0;
		}
		else if(errno)
			return 0;
	
		//This function has no way of knowing if DST is in effect or not.
		time.tm_isdst = -1;
		time.tm_year -= 1900;
		time.tm_mon--;
	
		//This function will normalize the 'struct tm' object.
		if
		(
			time.tm_year < 0
			||
			time.tm_mon < 0 || time.tm_mon > 11
			||
			time.tm_mday < 1
			||
			time.tm_hour < 0 || time.tm_hour > 23
			||
			time.tm_min < 0 || time.tm_min > 59
			||
			time.tm_sec < 0 || time.tm_sec > 60
		)
			return errno = CX_ERR_STR, 0;
	
		//If the current year is a leap year then the amount
		//of days in February is 29.
		if
		(
			time.tm_year % 4 == 0
			&&
			(time.tm_year % 100 != 0 || time.tm_year % 400 == 0)
		)
			mon[1]++;
	
		if(time.tm_mday > mon[time.tm_mon])
			return errno = CX_ERR_STR, 0;  
	
		//This loop sums together all the days of the months
		//before the current month.	
		for(; i < time.tm_mon; i++)
			time.tm_yday += mon[i];
	
		//Now just add the number of days, so far, in this month
		//and we have the day of the year. 'tm_yday' represents
		//the number of days since January 1st so we also subtract 1.
		time.tm_yday += time.tm_mday - 1;
	
		//yoff is the number of years since 1899.
		yoff = time.tm_year + 1;
	
		//This is formula counts the number of days since
		//January 1st, 1899 and then uses the modulo operator
		//to find the day of the week. This formula represents
		//Sunday through Saturday as 0-6 respectively.
		time.tm_wday = (yoff * 365 + yoff / 4 - yoff / 100
			            + yoff / 400 + time.tm_yday) % 7;
		
		*(struct tm*)obj = time;
		
		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_clock_t_strto(cxsize_t sizof, cxaddress_t addr, cxaddress_t obj, va_list *argv)
	{
		if(sizof != sizeof(clock_t) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t array = *(cxarray_t*)addr;
		long long int inte = 0;
		cxsize_t ret = 0;
		int tmperrno = errno;
		errno = 0;
	
		ret = cxui_ints_strto(sizeof(long long int), addr, &inte, argv);
	
		if(errno) return 0;
	
		*(clock_t*)obj = (clock_t)inte;
	
		return errno = tmperrno, ret;
	}

/*
	char 'tostr'
*/

	cxsize_t cxui_simple_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		const cxbyte_t *objb = obj;
		char *str = array->ptr;
		cxsize_t i = 0, j = 2;
		cxbyte_t nybble = 0;
		int pace = 1;
		CX_DEF_UNUSED(argv);

		//This statement enforces bounds-checking.
		if(2 * sizof + 3 > array->size) return errno = CX_ERR_SIZ, 0;

		//This function will always include a leading '0x'.
		str[0] = '0', str[1] = 'x';

		//This function will convert every single byte even if the
		//bytes are trailing zeros.
		for(; i < sizof; j++)
		{
			if(pace == 2) nybble = objb[i] & 0xF, pace = 1, i++;
			else nybble = (objb[i] & 0xF0) >> 4, pace++;
	
			if(nybble < 10) str[j] = '0' + nybble;
			else str[j] = 'A' + (nybble - 0xA);
		}

		str[j] = 0;
		array->len = -1;
	
		return j;
	}

	cxsize_t cxui_unsigned_ints_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		unsigned long long int inte = 0;
		int nchars = 0, tmperrno = errno;
		errno = 0;
		CX_DEF_UNUSED(argv);

		//This function will promote all integers to 'unsigned long long int'
		//before converting them to a string.
		if(sizof == sizeof(unsigned int))
			inte = *(unsigned int*)obj;
		else if(sizof == sizeof(unsigned long long int))
			inte = *(unsigned long long int*)obj;
		else if(sizof == sizeof(unsigned long int))
			inte = *(unsigned long int*)obj;
		else if(sizof == sizeof(unsigned short int))
			inte = *(unsigned short int*)obj;
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		//This function will use snprintf() to enforce bounds-checking.
		if((nchars = snprintf(array->ptr, array->size, "%llu", inte)) < 0)
		{
			if(!errno) errno = CX_ERR_OUT;
			return 0;
		}
		else if(nchars >= array->size)
			return errno = CX_ERR_SIZ, 0;
		else if(errno)
			return 0;

		array->len = -1;

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_ints_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		long long int inte = 0;
		int nchars = 0, tmperrno = errno;
		errno = 0;
		CX_DEF_UNUSED(argv);

		//This function will promote all integers to 'long long int'
		//before converting them to a string.
		if(sizof == sizeof(int))
			inte = *(int*)obj;
		else if(sizof == sizeof(long long int))
			inte = *(long long int*)obj;
		else if(sizof == sizeof(long int))
			inte = *(long int*)obj;
		else if(sizof == sizeof(short int))
			inte = *(short int*)obj;
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		//This function will use snprintf() to enforce bounds-checking.
		if((nchars = snprintf(array->ptr, array->size, "%lli", inte)) < 0)
		{
			if(!errno) errno = CX_ERR_OUT;
			return 0;
		}
		else if(nchars >= array->size)
			return errno = CX_ERR_SIZ, 0;
		else if(errno)
			return 0;

		array->len = -1;

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_floats_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof <= 0 || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		long double flt = 0.0L;
		char *str = array->ptr;
		cxsize_t pre = 0;
		int nchars = 0, tmperrno = errno;
	
		//This function will promote all floats to 'long double'
		//before converting them to a string.
		if(sizof == sizeof(double))
			flt = *(double*)obj;
		else if(sizof == sizeof(long double))
			flt = *(long double*)obj;
		else if(sizof == sizeof(float))
			flt = *(float*)obj;
		else
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}

		//This function will take as an argument an int
		//representing the precision, number of significant
		//digits, of the floating-point object. 
		if(argv == NULL) pre = CXUI_DEF_FLO_PRE;
		else pre = va_arg(*argv, int);
	
		//This function will use snprintf() to enforce bounds-checking.
		if((nchars = snprintf(str, array->size, "%.*Le", (int)pre, flt)) < 0)
		{
			if(!errno) errno = CX_ERR_OUT;
			return 0;
		}
		else if(nchars >= array->size)
			return errno = CX_ERR_SIZ, 0;
		else if(errno)
			return 0;
		
		array->len = -1;

		return errno = tmperrno, nchars;
	}

	cxsize_t cxui_char_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(char) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		CX_DEF_UNUSED(argv);
		
		if(array->size < 2) return errno = CX_ERR_SIZ, 0;

		memcpy(array->ptr, obj, sizeof(char));

		*(char*)(array->ptr + sizeof(char)) = 0;
		array->len = -1;

		return 1;
	}

	cxsize_t cxui_wchar_t_tostr(cxsize_t sizof, const cxaddress_t obj, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(wchar_t) || !obj || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		int b = 0;
		char c = 0;
		CX_DEF_UNUSED(argv);
		
		if((b = wctob(*(wchar_t*)obj) == EOF))
			return errno = CX_ERR_ARG, 0;
		
		c = (char)b;
		
		memcpy(array->ptr, &c, sizeof(char));
		
		*(char*)(array->ptr + sizeof(char)) = 0;
		array->len = -1;
		
		return 1;		
	}

	cxsize_t cxui_str_tostr(cxsize_t sizof, const cxaddress_t objp, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(cxarray_t) || !objp || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		const cxarray_t *obj = objp;
		cxsize_t len = 0;
		CX_DEF_UNUSED(argv);
	
		//A string to string conversion is just a string copy.
		if(!cxui_str_cpy(sizof, addr, objp) && errno)
			return 0;
	
		//This function will return the number of characters
		//copied to the destination string.
		if(obj->len == -1)
		{
			if(!(len = cxui_strlen(objp)) && errno)
				return 0;
		}
		else len = obj->len;
	
		return len;
	}

	cxsize_t cxui_time_t_tostr(cxsize_t sizof, const cxaddress_t objp, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(time_t) || !objp || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		struct tm time;
	
		//localtime() uses static storage to hold
		//the result of the function. This can
		//cause race-conditions. So this function
		//will use localtime_s() or localtime_r()
		//if available to prevent race-conditions.
		#if __STDC_WANT_LIB_EXT1__
	
			if(!localtime_s(objp, &time))
				return 0;
			
		#elif _POSIX_C_SOURCE >= 1
	
			if(!localtime_r(objp, &time))
				return 0;
			
		#else
	
			struct tm *timep;
	
			if(!(timep = localtime(objp)))
				return 0;
	
			time = *timep;
		
		#endif
	
		return cxui_struct_tm_tostr(sizeof(struct tm), &time, array, argv);
	}

	cxsize_t cxui_struct_tm_tostr(cxsize_t sizof, const cxaddress_t objp, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(struct tm) || !objp || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
		
		cxarray_t *array = addr;
		cxsize_t nchars;
		const struct tm *time = objp;
		CX_DEF_UNUSED(argv);
	
		//This is a table representing the days in each month of the year.
		int mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
		//This function will normalize the 'struct tm' object.
		if
		(
			time->tm_year < 0
			||
			time->tm_mon < 0 || time->tm_mon > 11
			||
			time->tm_mday < 1
			||
			time->tm_hour < 0 || time->tm_hour > 23
			||
			
			time->tm_min < 0 || time->tm_min > 59
			||
			time->tm_sec < 0 || time->tm_sec > 60
		)
			return errno = CX_ERR_ARG, 0;
	
		//If the current year is a leap year then the amount
		//of days in February is 29.
		if
		(
			time->tm_year % 4 == 0
			&&
			(time->tm_year % 100 != 0 || time->tm_year % 400 == 0)
		)
			mon[1]++;
	
		if(time->tm_mday > mon[time->tm_mon])
			return errno = CX_ERR_ARG, 0;  
	
		nchars = snprintf(array->ptr, array->size, "%i-%.2i-%.2iT%.2i:%.2i:%.2i",
		time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

		if(nchars < 0)
		{
			if(!errno) errno = CX_ERR_OUT;
			return 0;
		}
		else if(nchars >= array->size)
			return errno = CX_ERR_SIZ, 0;
		else if(errno)
			return 0;
	
		array->len = -1;
	
		return nchars;
	}

	cxsize_t cxui_clock_t_tostr(cxsize_t sizof, const cxaddress_t objp, cxaddress_t addr, va_list *argv)
	{
		if(sizof != sizeof(clock_t) || !objp || !cxui_verify(addr))
		{
			if(!errno) errno = CX_ERR_ARG;
			return 0;
		}
	
		//clock_t is defined as a standard arithmetic datatype
		//which could be a floating-point or an integer. This
		//function casts clock_t to an 'long long int' to
		//ensure a definite representation.
		clock_t obj = *(clock_t*)objp;
		long long int inte = (long long int)obj;
	
		return cxui_ints_tostr(sizeof(long long int), &inte, addr, argv);
	}

#endif
