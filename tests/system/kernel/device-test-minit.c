#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "card.h"
#include "dcs.h"
#include "wcnsls.h"
#include "ocnsls.h"
#include "kcore_pi.h"

static kword_t device_test_card[CARD_COLUMNS];
static int device_test_ptr_byte;

/* Guard adjacent KCORE state across real level-7 device interrupts. */
static kword_t device_test_boot_handoff_saved[2];

extern kword_t pdp10_pi_level4;
extern kword_t device_test_pi7_handler_address;
extern volatile kword_t device_test_pi7_done;

#define DEVICE_TEST_GUARD0 012345670123UL
#define DEVICE_TEST_GUARD1 076543210765UL

void
device_test_guard_minit(void)
{
        device_test_boot_handoff_saved[0] = kcore_boot_handoff[0];
        device_test_boot_handoff_saved[1] = kcore_boot_handoff[1];
        kcore_boot_handoff[0] = DEVICE_TEST_GUARD0;
        kcore_boot_handoff[1] = DEVICE_TEST_GUARD1;
}

static void
device_test_fail(void)
{
        for (;;)
                ;
}

static void
device_test_wait(unsigned int spins)
{
        volatile unsigned int remaining;

        remaining = spins;
        while (remaining != 0U)
                --remaining;
}

static void
device_test_clock(void)
{
        unsigned int address;
        kword_t before;
        unsigned int i;

        address = module_service_get(MODULE_SERVICE_CLK_TICKS);
        if (address == 0U)
                device_test_fail();
        before = kinit_call18_0(address);
        for (i = 0U; i < 040U; ++i) {
                device_test_wait(0200000U);
                if (kinit_call18_0(address) != before)
                        return;
        }
        device_test_fail();
}

void
device_test_nested_minit(void)
{
        kword_t level4_before;
        unsigned int handler;
        unsigned int i;

        handler = (unsigned int)device_test_pi7_handler_address;
        if (handler == 0U || module_pi_register(7U, handler) != 0)
                device_test_fail();
        device_test_pi7_done = 0;
        level4_before = pdp10_pi_level4;
        minit_pi_request((kword_t)PDP10_PI_MASK(7U));
        for (i = 0U; i < 010000U && device_test_pi7_done == 0; ++i)
                ;
        if (module_pi_unregister(7U, handler) != 0)
                device_test_fail();
        if (device_test_pi7_done == 0 || pdp10_pi_level4 == level4_before)
                device_test_fail();
}

void
device_test_minit(void)
{
        unsigned int address;
        int boot_handoff_corrupt;

        device_test_clock();

        address = module_service_get(MODULE_SERVICE_PTR_GETCHAR);
        if (address == 0U ||
            (int)kinit_call18_1(address,
                (kword_t)(unsigned long)&device_test_ptr_byte) != 0 ||
            device_test_ptr_byte != 0252)
                device_test_fail();

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

        address = module_service_get(MODULE_SERVICE_OCNSLS_READ);
        if (address == 0U)
                device_test_fail();
        (void)kinit_call18_0(address);

        /* DCS is disabled in the ordinary run.  The socket-backed variant
         * enables it, sends one byte, and verifies both resident services. */
        address = module_service_get(MODULE_SERVICE_DCS_GETCHAR);
        if (address != 0U) {
                static const unsigned int text[] = {
                        'D','A','I','M','O','S',' ','D','C','S','1',' ','O','K',015,012
                };
                kword_t rx;
                unsigned int putchar_address;
                unsigned int i;

                rx = kinit_call18_0(address);
                if (DCS_RX_LINE(rx) != 1U || DCS_RX_CHAR(rx) != 'R')
                        device_test_fail();
                putchar_address = module_service_get(MODULE_SERVICE_DCS_PUTCHAR);
                if (putchar_address == 0U)
                        device_test_fail();
                for (i = 0U; i < sizeof(text) / sizeof(text[0]); ++i) {
                        if ((int)kinit_call18_1(putchar_address,
                            DCS_PACK(1U, text[i])) != DCS_E_OK)
                                device_test_fail();
                }
        }

        boot_handoff_corrupt =
            kcore_boot_handoff[0] != DEVICE_TEST_GUARD0 ||
            kcore_boot_handoff[1] != DEVICE_TEST_GUARD1;
        kcore_boot_handoff[0] = device_test_boot_handoff_saved[0];
        kcore_boot_handoff[1] = device_test_boot_handoff_saved[1];
        if (boot_handoff_corrupt)
                device_test_fail();
}
