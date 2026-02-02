/*!
    @example

    \brief Examples of memory management
*/

#include <cxlib.h>

cxbyte_t* func();

int main()
{
    //Initialize the runtime
    __cxmain__

    //All memory stored in cxauto is deallocated on the next cxreturn
    cxaddress_t addr = cxmalloc(cxauto, 1000);

    //But memory can be moved to another storage after allocation.
    //All memory stored in cxstatic is deallocated when the program exits
    cxrestore(cxstatic, addr);

    //Note that addr may come from malloc or cxmalloc
    cxfree(addr);

    printf("%p\n", func());

    //You must use cxreturn not return
    cxreturn 0;
}

cxbyte_t* func()
{
    __cxfunc__

    //We can also return function local memory by pushing it before returning
    cxbyte_t* addr = cxpush(cxmalloc(cxauto, 1000));

    cxreturn addr;
}
