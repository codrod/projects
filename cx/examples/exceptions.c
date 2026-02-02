/*!
    @example

    \brief Examples of exception handling.
*/

#include <cxlib.h>

int func();

int main()
{
    __cxmain__

    cxtry
    {
        //All CX exceptions are derived from CXException
        cxthrow(CXException);
    }
    cxcatch()
    {
        //cxexcept is the excpetion caught above
        printf("caught %s\n", cxexcept.type);

        //continues
    }

    cxtry
    {
        cxbyte_t *p = NULL;

		cxbyte_t b = *p;
    }
    //Multiple exceptions can be caught including seg faults
    cxcatch(CXException_Signal_SegFault, CXException_Memory_NoMemory)
    {
        printf("caught %s\n", cxexcept.type);
    }

    cxtry
    {
        func();
    }
    cxcatch(CXException_Memory_NoMemory)
    {
        printf("caught %s\n", cxexcept.type);
    }

    cxreturn 0;
}

int func()
{
    //Initialize function
    __cxfunc__

    cxthrow(CXException_Memory_NoMemory);

    //Dont forget to use cxreturn not return
    cxreturn 0;
}
