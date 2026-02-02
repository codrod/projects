/*
	C Extension - Universal Interface II - Source Code
	Cody M. Rodgers
*/

/*
	change ret values 0 should be success non-zero error
*/

#include <cxui.h>

cxui_t cxui_binary = {"binary", 1, NULL, NULL, &cxui_binary_compare, &cxui_binary_copy, NULL, NULL};

cxui_t cxui_unsigned_short_int = {"unsigned short int", sizeof(unsigned short int), NULL, NULL, &cxui_binary_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_unsigned_int = {"unsigned int", sizeof(unsigned int), NULL, NULL, &cxui_binary_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_unsigned_long_int = {"unsigned long int", sizeof(unsigned long int), NULL, NULL, &cxui_binary_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_unsigned_long_long_int = {"unsigned long long int", sizeof(unsigned long long int), NULL, NULL, &cxui_binary_compare, &cxui_binary_copy, NULL, NULL};

cxui_t cxui_short_int = {"short int", sizeof(short int), NULL, NULL, &cxui_ints_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_int = {"int", sizeof(int), NULL, NULL, &cxui_ints_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_long_int = {"long int", sizeof(long int), NULL, NULL, &cxui_ints_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_long_long_int = {"long long int", sizeof(long long int), NULL, NULL, &cxui_ints_compare, &cxui_binary_copy, NULL, NULL};

cxui_t cxui_float = {"float", sizeof(float), NULL, NULL, &cxui_floats_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_double = {"double", sizeof(double), NULL, NULL, &cxui_floats_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_long_double = {"long double", sizeof(long double), NULL, NULL, &cxui_floats_compare, &cxui_binary_copy, NULL, NULL};

cxui_t cxui_char = {"char", sizeof(char), NULL, NULL, &cxui_char_compare, &cxui_binary_copy, NULL, NULL};

//cxui_t cxui_str = {"cxstr_t", sizeof(cxarray_t), NULL, NULL, &cxui_str_compare, &cxui_str_copy, &cxui_str_strto, &cxui_str_tostr};

cxui_t cxui_time_t = {"time_t", sizeof(time_t), NULL, NULL, &cxui_time_t_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_struct_tm = {"struct tm", sizeof(struct tm), NULL, NULL, &cxui_struct_tm_compare, &cxui_binary_copy, NULL, NULL};
cxui_t cxui_clock_t = {"clock_t", sizeof(clock_t), NULL, NULL, &cxui_clock_t_compare, &cxui_binary_copy, NULL, NULL};

/*
	Comparison
*/

int cxui_binary_compare(cxsize_t sizof, const cxaddress_t obj1, const cxaddress_t obj2, va_list *argv)
{
	if(sizof <= 0 || !obj1 || !obj2)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}

	int ret = memcmp(obj1, obj2, sizof);
	CX_UNUSED(argv);

	if(ret < 0) return CXUI_COM_LES;
	else if(ret > 0) return CXUI_COM_GRE;

	return CXUI_COM_EQU;
}

int cxui_ints_compare(cxsize_t sizof, const cxaddress_t obj1, const cxaddress_t obj2, va_list *argv)
{
	if(!obj1 || !obj2)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}	

	long long int int1 = 0, int2 = 0;
	CX_UNUSED(argv);

	//This function will promote all integers to 'long long int'.
	if(sizof == sizeof(int))
		int1 = *(int*)obj1, int2 = *(int*)obj2;
	else if(sizof == sizeof(long long int))
		int1 = *(long long*)obj1, int2 = *(long int*)obj2;
	else if(sizof == sizeof(long int))
		int1 = *(long int*)obj1, int2 = *(long int*)obj2;
	else if(sizof == sizeof(short int))
		int1 = *(short int*)obj1, int2 = *(short int*)obj2;
	else
	{
		if(!errno) errno = CXERROR_ARG;
		return 0;
	}
	
	if(int1 < int2) return CXUI_COM_LES;
	else if(int1 > int2) return CXUI_COM_GRE;

	return CXUI_COM_EQU;
}

#if !CXUI_NO_FLOAT_EQUAL

	int cxui_floats_equal(cxsize_t sizof, const cxaddress_t obj1, const cxaddress_t obj2, int pre, int base)
	{
		if(!obj1 || !obj2)
		{
			if(!errno) errno = CXERROR_ARG;
			return 0;
		}
	
		long double flt1 = 0.0L, flt2 = 0.0L;
		long long int exp1 = 0, exp2 = 0, exp3 = 0;
	
		//This function will promote all floats to 'long double'.
		if(sizof == sizeof(double))
			flt1 = *(double*)obj1, flt2 = *(double*)obj2;
		else if(sizof == sizeof(long double))
			flt1 = *(long double*)obj1, flt2 = *(long double*)obj2;
		else if(sizof == sizeof(float))
			flt1 = *(float*)obj1, flt2 = *(float*)obj2;
		else
		{
			if(!errno) errno = CXERROR_ARG;
			return 0;
		}

		//The floats are exactly equal.
		if(flt1 == flt2) return 1;
	
		//This catches 754 NaNs without explicitly saying NaN.
		if(flt1 != flt1)
		{
			//This means NaN equals NaN.
			if(flt2 != flt2) return 1;
			else return 0;
		}
	
		//This catches 754 infinites without explicitly saying infinity. Note that
		//we already know that flt1 != flt2 because of the statement above.
		if(flt1 == HUGE_VALL || flt1 == -HUGE_VALL || flt2 == HUGE_VALL || flt2 == -HUGE_VALL)
			return 0;

		//If the signs do not match then they are not equal.
		if((flt1 < 0.0L && flt2 > 0.0L) || (flt1 > 0.0L && flt2 < 0.0L))
			return 0;
		
		flt1 = fabsl(flt1), flt2 = fabsl(flt2);
		
		//The logarithm of a number is also the exponent of that number
		//written in normalized exponential notation.
		exp1 = (long long int)(log10l(flt1) / log10l((long double)base));
		exp2 = (long long int)(log10l(flt2) / log10l((long double)base));
		
		//If the numbers are not equal then their exponents will not be equal.
		if(exp1 != exp2) return 0;
		
		exp1 = llabs(exp1), exp2 = llabs(exp2);
		
		//If some of the leading digits in the mantissa of flt1 and flt2 are equal then the exponent
		//of their difference will decrease by the number of equal leading digits.
		exp3 = llabs((long long int)(log10l(fabsl(flt1 - flt2)) / log10l((long double)base)));
		
		//The difference between exp1 and exp3 should equal the number
		//of leading digits that are equal in flt1 and flt2.
		if(exp1 - exp3 < pre) return 0;
		
		return 1;
	}

#endif

int cxui_floats_compare(cxsize_t sizof, const cxaddress_t obj1, const cxaddress_t obj2, va_list *argv)
{
	if(!obj1 || !obj2)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}

	long double flt1 = 0.0L, flt2 = 0.0L;
	int pre = 0, base = 0;
	
	
	//This function takes two arguments. The first argument is
	//the precision, number of significant digits. The second
	//argument represents the base of the significant digits.
	//For example (..., 6, 10) is six decimal digits and (..., 6, 16)
	//is six hexadecimal digits.
	if(argv)
	{
		pre = va_arg(*argv, int);
		base = va_arg(*argv, int);
	}
	else pre = CXUI_DEFAULT_FLOAT_PRECISION, base = CXUI_DEFAULT_FLOAT_BASE;

	//This function promotes all floats to long double.
	if(sizof == sizeof(double))
		flt1 = *(double*)obj1, flt2 = *(double*)obj2;
	else if(sizof == sizeof(long double))
		flt1 = *(long double*)obj1, flt2 = *(long double*)obj2;
	else if(sizof == sizeof(float))
		flt1 = *(float*)obj1, flt2 = *(float*)obj2;
	else
	{
		if(!errno) errno = CXERROR_ARG;
		return 0;
	}
	
	if(cxui_floats_equal(sizof, obj1, obj2, pre, base)) return CXUI_COM_EQU;
	else if(flt1 < flt2) return CXUI_COM_LES;

	return CXUI_COM_GRE;
}

int cxui_char_compare(cxsize_t sizof, const cxaddress_t obj1, const cxaddress_t obj2, va_list *argv)
{
	if(sizof != sizeof(char) || !obj1 || !obj2)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}

	int ret = 0, tmperrno = errno;
	errno = 0;
	CX_UNUSED(argv);
	 
	ret = strncmp(obj1, obj2, 1);
	
	if(errno) return CXUI_COM_ERR;

	if(ret > 0) return errno = tmperrno, CXUI_COM_GRE;
	else if(ret < 0) return errno = tmperrno, CXUI_COM_LES;

	return errno = tmperrno, CXUI_COM_EQU;
}

/*
int cxui_str_compare(cxsize_t sizof, const cxaddress_t obj1p, const cxaddress_t obj2p, va_list *argv)
{
	if(sizof != sizeof(cxarray_t) || !obj1p || !obj2p)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}

	const cxarray_t *obj1 = obj1p, *obj2 = obj2p;
	cxsize_t n = 0, len1 = 0, len2 = 0;
	int ret = 0, tmperrno = errno;
	errno = 0;
	CX_UNUSED(argv);

	//If obj1->len equals -1 then the string is
	//null terminated so this function will enforce
	//bounds checking using obj1->size.
	if(obj1->len == -1) len1 = obj1->size;
	else len1 = obj1->len;
	//Else then the string is not necessarily
	//null terminated. So this function interprets
	//obj1->len as the 'n' argument in strncmp().
	//And since strncmp() always interprets null-characters
	//this function does as well.

	if(obj2->len == -1) len2 = obj2->size;
	else len2 = obj2->len;

	//This function will compare the minimum of len1 and len2
	//characters.
	if(len1 <= len2) n = len1;
	else n = len2;

	ret = strncmp(obj1->ptr, obj2->ptr, n);

	if(errno) return CXUI_COM_ERR;

	if(ret < 0) return errno = tmperrno, CXUI_COM_LES;
	else if(ret > 0) return errno = tmperrno, CXUI_COM_GRE;

	return errno = tmperrno, CXUI_COM_EQU;
}
*/

int cxui_time_t_compare(cxsize_t sizof, const cxaddress_t obj1p, const cxaddress_t obj2p, va_list *argv)
{
	if(sizof != sizeof(time_t) || !obj1p || !obj2p)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}
	
	const time_t obj1 = *(time_t*)obj1p, obj2 = *(time_t*)obj2p;
	CX_UNUSED(argv);
	
	if(obj1 > obj2) return CXUI_COM_GRE;
	else if(obj1 < obj2) return CXUI_COM_LES;
	
	return CXUI_COM_EQU;
}

int cxui_struct_tm_compare(cxsize_t sizof, const cxaddress_t obj1p, const cxaddress_t obj2p, va_list *argv)
{
	if(sizof != sizeof(struct tm) || !obj1p || !obj2p)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}
	
	time_t time1, time2;
	CX_UNUSED(argv);
	
	if
	(
		((time1 = mktime((struct tm*)obj1p)) == (time_t)(-1))
		||
		((time2 = mktime((struct tm*)obj2p)) == (time_t)(-1))
	)
		return errno = CXERROR_ARG, CXUI_COM_ERR;
	
	return cxui_time_t_compare(sizeof(time_t), &time1, &time2, argv);
}

int cxui_clock_t_compare(cxsize_t sizof, const cxaddress_t obj1p, const cxaddress_t obj2p, va_list *argv)
{
	if(sizof != sizeof(clock_t) || !obj1p || !obj2p)
	{
		if(!errno) errno = CXERROR_ARG;
		return CXUI_COM_ERR;
	}
	
	const clock_t obj1 = *(clock_t*)obj1p, obj2 = *(clock_t*)obj2p;
	CX_UNUSED(argv);
	
	if(obj1 > obj2) return CXUI_COM_GRE;
	else if(obj1 < obj2) return CXUI_COM_LES;
	
	return CXUI_COM_EQU;
}

/*
	Copy
*/

int cxui_binary_copy(cxsize_t sizof, cxaddress_t des, const cxaddress_t src)
{
	if(sizof <= 0 || !des || !src)
	{
		if(!errno) errno = CXERROR_ARG;
		return 0;
	}

	memcpy(des, src, sizof);

	return 1;
}

/*
int cxui_str_copy(cxsize_t sizof, cxaddress_t desp, const cxaddress_t srcp)
{
	if(sizof != sizeof(cxarray_t) || !desp || !srcp)
	{
		if(!errno) errno = CXERROR_ARG;
		return 0;
	}

	const cxarray_t *src = srcp;
	cxarray_t *des = desp;
	cxsize_t len = 0;
	
	if(!cxui_verify(srcp) || !cxui_verify(desp) || des->type->sizof != src->type->sizof)
	{
		if(!errno) errno = CXERROR_ARG;
		return 0;
	}

	if(src->len == -1)
	{
		if(!(len = cxui_strlen(srcp)) && errno)
			return 0;
	}
	else len = src->len;

	//This function will not copy more than des->size.
	if(len >= des->size - 1)
		return errno = CXERROR_SIZ, 0;
		
	//This function does not use strncpy() so it does not interpret
	//null characters as a delimiter; unless src->len equals -1.
	memcpy(des->ptr, src->ptr, len * sizeof(char));
	
	//This function will set the destination string's length.
	if(src->len == -1) des->len = -1;
	else des->len = len;

	return 1;
}
*/











