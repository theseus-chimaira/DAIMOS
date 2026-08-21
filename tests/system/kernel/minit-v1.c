/* minit-v1.c -- in-place MINIT call/return checkpoint. */

typedef int (*test_minit_putchar_fn_v1)(int c);

#define TEST_MINIT_KCORE_PUTCHAR_V1 000062UL

static void
test_minit_putc_v1(int c)
{
        test_minit_putchar_fn_v1 putfn;

        putfn = (test_minit_putchar_fn_v1)(unsigned long)
            TEST_MINIT_KCORE_PUTCHAR_V1;
        (void)(*putfn)(c);
}

void
test_minit_v1(void)
{
        static const char text[] = "MINIT TEST";
        unsigned int i;

        for (i = 0U; text[i] != '\0'; i++)
                test_minit_putc_v1((int)text[i]);
        test_minit_putc_v1(015);
        test_minit_putc_v1(012);
}
