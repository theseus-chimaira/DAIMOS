#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "card.h"
#include "dcs.h"
#include "ge.h"
#include "tty.h"
#include "wcnsls.h"
#include "ocnsls.h"
#include "kcore_pi.h"
#include "storage.h"

static kword_t device_test_card[CARD_COLUMNS];
static kword_t device_test_dsk_sector[DSK_WORDS_PER_SECTOR];
static kword_t device_test_mtc_record[8];
static kword_t device_test_dtc_record[0200];
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

        address = module_service_get(MODULE_SERVICE_TTY_PUTCHAR);
        if (address == 0U ||
            (int)kinit_call18_1(address, TTY_PACK(TTY_ID_CTY, 0U)) !=
                TTY_E_OK ||
            (int)kinit_call18_1(address, TTY_PACK(077U, 0U)) !=
                TTY_E_INVALID)
                device_test_fail();

        /* The DTC-enabled variant exposes four known words at the start of
         * unit 0.  Read exactly those words through the same resident PI5
         * transfer engine used by MTC and DSK. */
        address = module_service_get(MODULE_SERVICE_DTC_READ_WORDS);
        if (address != 0U) {
                unsigned int i;

                for (i = 0U; i < 4U; ++i)
                        device_test_dtc_record[i] = 0;
                if ((int)kinit_call18_3(address, 0UL,
                    (kword_t)(unsigned long)device_test_dtc_record, 4UL) != 0 ||
                    device_test_dtc_record[0] != 012345670123UL ||
                    device_test_dtc_record[1] != 076543210765UL ||
                    device_test_dtc_record[2] != 000000000001UL ||
                    device_test_dtc_record[3] != 0777777777776UL)
                        device_test_fail();

                for (i = 0U; i < 0200U; ++i)
                        device_test_dtc_record[i] = (kword_t)i;
                address = module_service_get(MODULE_SERVICE_DTC_WRITE_BLOCK);
                if (address == 0U ||
                    (int)kinit_call18_2(address, 0UL,
                    (kword_t)(unsigned long)device_test_dtc_record) != 0)
                        device_test_fail();
        }

        /* The MTC-enabled variant attaches one four-word record to unit 0.
         * Read it through the resident shared DCT/PI5 path and leave the
         * sentinel beyond EOR untouched. */
        address = module_service_get(MODULE_SERVICE_MTC_READ_WORDS);
        if (address != 0U) {
                unsigned int i;

                for (i = 0U; i < 8U; ++i)
                        device_test_mtc_record[i] = 0777777777777UL;
                if ((int)kinit_call18_3(address, 0UL,
                    (kword_t)(unsigned long)device_test_mtc_record, 8UL) != 0 ||
                    device_test_mtc_record[0] != 012345670123UL ||
                    device_test_mtc_record[1] != 076543210765UL ||
                    device_test_mtc_record[2] != 000000000001UL ||
                    device_test_mtc_record[3] != 0777777777776UL ||
                    device_test_mtc_record[4] != 0777777777777UL)
                        device_test_fail();

                device_test_mtc_record[0] = 011223344556UL;
                device_test_mtc_record[1] = 066554433221UL;
                address = module_service_get(MODULE_SERVICE_MTC_WRITE_WORDS);
                if (address == 0U ||
                    (int)kinit_call18_3(address, 0UL,
                    (kword_t)(unsigned long)device_test_mtc_record, 2UL) != 0)
                        device_test_fail();
        }

        /* Sector zero of mkdsk clean media is the compact DBC descriptor. */
        address = module_service_get(MODULE_SERVICE_DSK_READ_SECTOR);
        if (address == 0U ||
            (int)kinit_call18_2(address, 0UL,
                (kword_t)(unsigned long)device_test_dsk_sector) != 0 ||
            ((device_test_dsk_sector[0] >> 18) & 0777777UL) != 0444243UL)
                device_test_fail();

        {
                unsigned int i;
                unsigned int write_address;

                for (i = 0U; i < DSK_WORDS_PER_SECTOR; ++i)
                        device_test_dsk_sector[i] = 012345600000UL + i;
                write_address = module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR);
                if (write_address == 0U ||
                    (int)kinit_call18_2(write_address, 02713UL,
                        (kword_t)(unsigned long)device_test_dsk_sector) != 0)
                        device_test_fail();
                for (i = 0U; i < DSK_WORDS_PER_SECTOR; ++i)
                        device_test_dsk_sector[i] = 0;
                if ((int)kinit_call18_2(address, 02713UL,
                    (kword_t)(unsigned long)device_test_dsk_sector) != 0)
                        device_test_fail();
                for (i = 0U; i < DSK_WORDS_PER_SECTOR; ++i) {
                        if (device_test_dsk_sector[i] != 012345600000UL + i)
                                device_test_fail();
                }
        }

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
                if ((int)kinit_call18_1(putchar_address,
                    DCS_PACK(1U, text[0])) != DCS_E_OK)
                        device_test_fail();
                putchar_address = module_service_get(MODULE_SERVICE_TTY_PUTCHAR);
                if (putchar_address == 0U)
                        device_test_fail();
                for (i = 1U; i < sizeof(text) / sizeof(text[0]); ++i) {
                        if ((int)kinit_call18_1(putchar_address,
                            TTY_PACK(TTY_ID_DCS_BASE + 1U, text[i])) !=
                                TTY_E_OK)
                                device_test_fail();
                }
        }


        /* GE socket variant enables GE (and DCS simultaneously), verifies
         * GTY input, raw output, and TTY routing through the shared PI4
         * handler without requiring another KCORE PI-table slot. */
        address = module_service_get(MODULE_SERVICE_GE_GETCHAR);
        if (address != 0U) {
                static const unsigned int text[] = {
                        'D','A','I','M','O','S',' ','G','E','1',' ','O','K',015,012
                };
                kword_t rx;
                unsigned int putchar_address;
                unsigned int i;

                rx = kinit_call18_0(address);
                if (GE_RX_LINE(rx) != 1U || GE_RX_CHAR(rx) != 'G')
                        device_test_fail();
                putchar_address = module_service_get(MODULE_SERVICE_GE_PUTCHAR);
                if (putchar_address == 0U ||
                    (int)kinit_call18_1(putchar_address,
                    GE_PACK(1U, text[0])) != GE_E_OK)
                        device_test_fail();
                putchar_address = module_service_get(MODULE_SERVICE_TTY_PUTCHAR);
                if (putchar_address == 0U)
                        device_test_fail();
                for (i = 1U; i < sizeof(text) / sizeof(text[0]); ++i) {
                        if ((int)kinit_call18_1(putchar_address,
                            TTY_PACK(TTY_ID_GE_BASE + 1U, text[i])) !=
                                TTY_E_OK)
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
