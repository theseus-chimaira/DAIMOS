#include "dsys.h"

void
exit(int status)
{
        (void)dsys_exit(status);
        for (;;)
                ;
}

int
isdigit(int ch)
{
        return ch >= '0' && ch <= '9';
}
