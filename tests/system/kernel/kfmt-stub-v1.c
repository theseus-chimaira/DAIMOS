#include "kfmt_v1.h"
#include "kfmt-stub-v1.h"
kword_t kfmt_stub_value;
kword_t kfmt_stub_off;
unsigned int kfmt_stub_calls;
unsigned int kfmt_stub_ch = '?';
int kfmt_stub_rc = 1;
void
kfmt_stub_reset(void)
{
        kfmt_stub_value = 0;
        kfmt_stub_off = 0;
        kfmt_stub_calls = 0U;
        kfmt_stub_ch = '?';
        kfmt_stub_rc = 1;
}
int
kfmt_u36_decimal_readchar(kword_t value, kword_t off, unsigned int *chp)
{
        kfmt_stub_value = value;
        kfmt_stub_off = off;
        ++kfmt_stub_calls;
        if (chp != 0)
                *chp = kfmt_stub_ch;
        return kfmt_stub_rc;
}
