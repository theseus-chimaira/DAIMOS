#include "kinit.h"
#include "module.h"
#include "mres.h"
#include "kcore_io.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "pt.h"
#include "card.h"
#include "wcnsls.h"

#define CTY_X_HANDLER           0U
#define CTY_X_PUT6              1U
#define CTY_X_NEWLINE           2U
#define CTY_X_SPACES            3U

#define CLK_X_HANDLER           0U
#define PTR_X_HANDLER           0U
#define PTR_X_GETCHAR           1U
#define PTP_X_HANDLER           0U
#define PTP_X_PUTCHAR           1U
#define CR_X_HANDLER            0U
#define CR_X_READ_CARD          1U
#define CP_X_HANDLER            0U
#define CP_X_PUNCH_CARD         1U
#define WCNSLS_X_READ           0U

#define SLV_PI_MASK             0000007UL
#define SLV_CO_CLEAR_IRQ        0000010UL
#define SLV_PROBE_PI            7U

static unsigned int diag_put6_addr;
static unsigned int diag_newline_addr;
static unsigned int diag_spaces_addr;

static int
minit_put6(kword_t word)
{
        if (diag_put6_addr == 0U) {
                kinit_put6(word);
                return 0;
        }
        return (int)kinit_call18_1(diag_put6_addr, word);
}

static int
minit_spaces(unsigned int words)
{
        if (diag_spaces_addr == 0U) {
                kinit_put6_spaces(words);
                return 0;
        }
        return (int)kinit_call18_1(diag_spaces_addr, (kword_t)words);
}

static int
minit_newline(void)
{
        if (diag_newline_addr == 0U) {
                kinit_newline();
                return 0;
        }
        return (int)kinit_call18_0(diag_newline_addr);
}

static void
minit_output_failure(void)
{
        kinit_halt();
}

static void
minit_diag_ok(kword_t name)
{
        if (minit_put6(name) != 0 || minit_spaces(5U) != 0 ||
            minit_put6((kword_t)SIXBIT("  OK  ")) != 0 ||
            minit_newline() != 0)
                minit_output_failure();
}

static void
minit_diag_status6(kword_t name, kword_t first, kword_t second)
{
        if (minit_put6(name) != 0 || minit_spaces(4U) != 0 ||
            minit_put6(first) != 0 || minit_put6(second) != 0 ||
            minit_newline() != 0)
                minit_output_failure();
}

static void
minit_diag_nodev(kword_t name)
{
        minit_diag_status6(name, (kword_t)SIXBIT("    NO"),
            (kword_t)SIXBIT(" DEV  "));
}

static void
minit_diag_notok(kword_t name)
{
        minit_diag_status6(name, (kword_t)SIXBIT("    NO"),
            (kword_t)SIXBIT("T OK  "));
}

static void
minit_diag_nodrv(kword_t name)
{
        minit_diag_status6(name, (kword_t)SIXBIT("    NO"),
            (kword_t)SIXBIT(" DRV  "));
}

static void
minit_diag_name(kword_t name)
{
        if (minit_put6(name) != 0 || minit_newline() != 0)
                minit_output_failure();
}

static void
minit_diag_hz(void)
{
        if (minit_put6((kword_t)SIXBIT("HZ    ")) != 0 ||
            minit_spaces(4U) != 0 ||
            minit_put6((kword_t)SIXBIT("   60 ")) != 0 ||
            minit_put6((kword_t)SIXBIT("LINE  ")) != 0 ||
            minit_newline() != 0)
                minit_output_failure();
}

static void
minit_fatal(kword_t name)
{
        minit_diag_status6(name, (kword_t)SIXBIT("    FA"),
            (kword_t)SIXBIT("IL    "));
        kinit_halt();
}

static unsigned int
minit_install(kword_t name)
{
        const kword_t *package;
        unsigned int base;

        package = module_current_mres();
        if (package == 0 || mres_install(package, &base) != 0)
                minit_fatal(name);
        return base;
}

static unsigned int
minit_export(kword_t name, unsigned int base, unsigned int index)
{
        unsigned int address;

        address = mres_export(module_current_mres(), base, index);
        if (address == 0U)
                minit_fatal(name);
        return address;
}

static void
minit_register(kword_t name, unsigned int level, unsigned int handler)
{
        if (pdp10_pi_register(level, handler, 0) != 0)
                minit_fatal(name);
        pdp10_pi_hw_enable(PDP10_PI_MASK(level));
}

void
cty_minit(void)
{
        kword_t name;
        kword_t st;
        unsigned int base;

        name = (kword_t)SIXBIT("CTY   ");
        minit_cty_cono(CTY_NATIVE_PI_LEVEL);
        st = minit_cty_coni();
        if ((st & CTY_ST_PI_MASK) != CTY_NATIVE_PI_LEVEL) {
                minit_cty_cono(0);
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, CTY_NATIVE_PI_LEVEL,
            minit_export(name, base, CTY_X_HANDLER));
        minit_cty_cono(CTY_NATIVE_PI_LEVEL);
        diag_put6_addr = minit_export(name, base, CTY_X_PUT6);
        diag_newline_addr = minit_export(name, base, CTY_X_NEWLINE);
        diag_spaces_addr = minit_export(name, base, CTY_X_SPACES);
        minit_diag_ok(name);
}

void
clk_minit(void)
{
        kword_t st;
        unsigned int base;
        kword_t name;

        name = (kword_t)SIXBIT("HZ    ");
        minit_clk_cono((kword_t)CLK_NATIVE_PI_LEVEL | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        st = minit_clk_coni();
        if ((st & CLK_APR_ST_ENABLE) == 0) {
                minit_clk_cono(CLK_APR_CO_DISABLE | CLK_APR_CO_CLEAR_FLAG);
                minit_diag_notok(name);
                return;
        }
        minit_clk_cono(CLK_APR_CO_DISABLE | CLK_APR_CO_CLEAR_FLAG);
        base = minit_install(name);
        minit_register(name, CLK_NATIVE_PI_LEVEL,
            minit_export(name, base, CLK_X_HANDLER));
        minit_clk_cono((kword_t)CLK_NATIVE_PI_LEVEL | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        minit_diag_hz();
}

void
ptr_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("PTR   ");
        minit_ptr_cono(PT_NATIVE_PI_LEVEL);
        if ((minit_ptr_coni() & PT_ST_PI_MASK) != PT_NATIVE_PI_LEVEL) {
                minit_ptr_cono(0);
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, PT_NATIVE_PI_LEVEL,
            minit_export(name, base, PTR_X_HANDLER));
        minit_ptr_cono(PT_NATIVE_PI_LEVEL);
        minit_diag_ok(name);
}

void
ptp_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("PTP   ");
        minit_ptp_cono(PT_NATIVE_PI_LEVEL);
        if ((minit_ptp_coni() & PT_ST_PI_MASK) != PT_NATIVE_PI_LEVEL) {
                minit_ptp_cono(0);
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, PT_NATIVE_PI_LEVEL,
            minit_export(name, base, PTP_X_HANDLER));
        minit_ptp_cono(PT_NATIVE_PI_LEVEL);
        module_service_set(MODULE_SERVICE_PTP_PUTCHAR,
            minit_export(name, base, PTP_X_PUTCHAR));
        minit_diag_ok(name);
}

void
cr_minit(void)
{
        kword_t name;
        kword_t st;
        unsigned int base;

        name = (kword_t)SIXBIT("CR    ");
        minit_cr_cono(CR_CO_CLR_READER);
        minit_cr_cono((kword_t)CARD_NATIVE_PI_LEVEL | CR_CO_CLR_DRDY |
            CR_CO_CLR_END_CARD | CR_CO_CLR_DATA_MISS);
        st = minit_cr_coni();
        if ((st & CR_ST_PI_MASK) != CARD_NATIVE_PI_LEVEL) {
                minit_cr_cono(CR_CO_CLR_READER);
                minit_diag_nodev(name);
                return;
        }
        if ((st & CR_ST_TROUBLE) != 0) {
                minit_cr_cono(CR_CO_CLR_READER);
                minit_diag_notok(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, CARD_NATIVE_PI_LEVEL,
            minit_export(name, base, CR_X_HANDLER));
        minit_cr_cono((kword_t)CARD_NATIVE_PI_LEVEL | CR_CO_CLR_DRDY |
            CR_CO_CLR_END_CARD | CR_CO_CLR_DATA_MISS);
        module_service_set(MODULE_SERVICE_CR_READ_CARD,
            minit_export(name, base, CR_X_READ_CARD));
        minit_diag_ok(name);
}

void
cp_minit(void)
{
        kword_t name;
        kword_t st;
        unsigned int base;

        name = (kword_t)SIXBIT("CP    ");
        minit_cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_CLR_PUNCH);
        st = minit_cp_coni();
        if ((st & CP_ST_PI_MASK) != CARD_NATIVE_PI_LEVEL) {
                minit_cp_cono(CP_CO_CLR_PUNCH);
                minit_diag_nodev(name);
                return;
        }
        if ((st & (CP_ST_ERROR | CP_ST_TROUBLE)) != 0) {
                minit_cp_cono(CP_CO_CLR_PUNCH);
                minit_diag_notok(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, CARD_NATIVE_PI_LEVEL,
            minit_export(name, base, CP_X_HANDLER));
        minit_cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_CLR_PUNCH);
        module_service_set(MODULE_SERVICE_CP_PUNCH_CARD,
            minit_export(name, base, CP_X_PUNCH_CARD));
        minit_diag_ok(name);
}

void
wcnsls_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("WCNSLS");
        base = minit_install(name);
        module_service_set(MODULE_SERVICE_WCNSLS_READ,
            minit_export(name, base, WCNSLS_X_READ));
        minit_wcnsls_cono(WCNSLS_CO_SPACEWAR);
        minit_diag_name(name);
}

void
slv_minit(void)
{
        kword_t st;
        kword_t name;

        name = (kword_t)SIXBIT("SLV   ");
        minit_slv_cono(SLV_PROBE_PI);
        st = minit_slv_coni();
        minit_slv_cono(SLV_CO_CLEAR_IRQ);
        if ((st & SLV_PI_MASK) == SLV_PROBE_PI)
                minit_diag_nodrv(name);
        else
                minit_diag_nodev(name);
}
