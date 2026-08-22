#include "kinit.h"
#include "module.h"
#include "card.h"
#include "wcnsls.h"

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
        unsigned int address;

        address = module_service_get(MODULE_SERVICE_PTP_PUTCHAR);
        if (address != 0U && (int)kinit_call18_1(address, 0252UL) != 0)
                device_test_fail();

        address = module_service_get(MODULE_SERVICE_CR_READ_CARD);
        if (address == 0U ||
            (int)kinit_call18_1(address,
                (kword_t)(unsigned long)device_test_card) != (int)CARD_COLUMNS)
                device_test_fail();

        address = module_service_get(MODULE_SERVICE_CP_PUNCH_CARD);
        if (address == 0U ||
            (int)kinit_call18_1(address,
                (kword_t)(unsigned long)device_test_card) != (int)CARD_COLUMNS)
                device_test_fail();

        address = module_service_get(MODULE_SERVICE_WCNSLS_READ);
        if (address == 0U)
                device_test_fail();
        (void)kinit_call18_0(address);
}
