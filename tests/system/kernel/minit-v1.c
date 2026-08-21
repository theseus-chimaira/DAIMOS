/* minit-v1.c -- driver-free in-place MINIT call/return checkpoint. */

typedef unsigned long test_word_t_v2;
typedef void (*test_minit_put6_fn_v2)(test_word_t_v2 word);

#define TEST_MINIT_BOOT_PUT6_V2 077760UL
#define TEST_MINIT_SIXBIT_V2(c) ((((test_word_t_v2)(c)) - 040UL) & 077UL)
#define TEST_MINIT_W6_V2(a,b,c,d,e,f) \
    ((TEST_MINIT_SIXBIT_V2(a) << 30) | (TEST_MINIT_SIXBIT_V2(b) << 24) | \
     (TEST_MINIT_SIXBIT_V2(c) << 18) | (TEST_MINIT_SIXBIT_V2(d) << 12) | \
     (TEST_MINIT_SIXBIT_V2(e) << 6) | TEST_MINIT_SIXBIT_V2(f))

static void
test_minit_put6_v2(test_word_t_v2 word)
{
        test_minit_put6_fn_v2 putfn;

        putfn = (test_minit_put6_fn_v2)(unsigned long)TEST_MINIT_BOOT_PUT6_V2;
        (*putfn)(word);
}

void
test_minit_v1(void)
{
        test_minit_put6_v2(TEST_MINIT_W6_V2('M','I','N','I','T',' '));
        test_minit_put6_v2(TEST_MINIT_W6_V2('T','E','S','T',' ',' '));
}
