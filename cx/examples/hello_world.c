/*!
    @example

    \brief A hello world example.
*/

#include <cxlib.h>

int main()
{
    //Initialize the runtime
    __cxmain__

    printf("Hello World!\n");

    //You must use cxreturn not return
    cxreturn 0;
}
