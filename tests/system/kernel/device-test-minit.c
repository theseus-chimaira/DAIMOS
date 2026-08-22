#include "pt.h"
#include "card.h"
#include "joy.h"

static kword_t device_test_card[CARD_COLUMNS];

static void
device_test_fail(void)
{
        for (;;)
                ;
}

void
device_test_minit(void)
{
        if (ptp_putchar(0252) != PT_E_OK)
                device_test_fail();
        if (cr_read_card(device_test_card) != (int)CARD_COLUMNS)
                device_test_fail();
        if (cp_punch_card(device_test_card) != (int)CARD_COLUMNS)
                device_test_fail();
        if ((wcnsls_read() & WCNSLS_WORD_MASK) != WCNSLS_WORD_MASK)
                device_test_fail();
        if ((ocnsls_read() & OCNSLS_WORD_MASK) != 0)
                device_test_fail();
}
