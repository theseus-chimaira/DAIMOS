#include "kinit.h"

extern kword_t test_resident_word_v1;

void
test_minit_v1(void)
{
        test_resident_word_v1 = 0123456UL;
        kinit_put6(SIXBIT("MINIT1"));
        kinit_newline();
}

void
test_minit2_v1(void)
{
        if (test_resident_word_v1 == 0123456UL)
                kinit_put6(SIXBIT("MINIT2"));
        else
                kinit_put6(SIXBIT("?MRES "));
        kinit_newline();
}
