#include "dsys.h"

void
exit(int status)
{
        (void)dsys_exit(status);
        for (;;)
                ;
}

/* A genuine abnormal process termination, not a test-only trap. */
void
abort(void)
{
        exit(134);
}

int
isdigit(int ch)
{
        return ch >= '0' && ch <= '9';
}

int
isspace(int ch)
{
        return ch == ' ' || ch == '\t' || ch == '\n' ||
            ch == '\r' || ch == '\f' || ch == '\v';
}
