#include "kinit.h"
#include "module.h"
#include "mres.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "pt.h"
#include "card.h"
#include "dcs.h"
#include "tty.h"
#include "wcnsls.h"
#include "ocnsls.h"

#define CTY_X_HANDLER           0U
#define CTY_X_PUT6              1U
#define CTY_X_PUTCHAR           2U

#define CLK_X_HANDLER           0U
#define CLK_X_TICKS             1U
#define PTR_X_HANDLER           0U
#define PTR_X_GETCHAR           1U
#define PTP_X_HANDLER           0U
#define PTP_X_PUTCHAR           1U
#define CR_X_HANDLER            0U
#define CR_X_READ_CARD          1U
#define CP_X_HANDLER            0U
#define CP_X_PUNCH_CARD         1U
#define DCS_X_HANDLER           0U
#define DCS_X_GETCHAR           1U
#define DCS_X_PUTCHAR           2U
#define TTY_X_PUTCHAR           0U
#define TTY_X_CTY_PUTCHAR_ADDR  1U
#define TTY_X_DCS_PUTCHAR_ADDR  2U
#define WCNSLS_X_READ           0U
#define OCNSLS_X_READ           0U

#define SLV_PI_MASK             0000007UL
#define SLV_CO_CLEAR_IRQ        0000010UL
#define SLV_PROBE_PI            7U

static unsigned int diag_put6_addr;
static unsigned int diag_putchar_addr;


static unsigned int pi_level_count[PDP10_PI_LEVELS + 1U];
static unsigned int pi_handler_total;
static unsigned int pi_enabled_mask;

static kword_t
minit_pi_span(unsigned int start, unsigned int count)
{
        kword_t neg_count;

        if (count == 0U)
                return 0;
        neg_count = (kword_t)((01000000U - count) & 0777777U);
        return (neg_count << 18) | (kword_t)(start & 0777777U);
}

static void
minit_pi_reindex(void)
{
        unsigned int level;
        unsigned int start;

        start = 0U;
        for (level = PDP10_PI_LEVEL_MIN; level <= PDP10_PI_LEVEL_MAX;
            ++level) {
                pdp10_pi_level_span[level - 1U] =
                    minit_pi_span(start, pi_level_count[level]);
                start += pi_level_count[level];
        }
}

void
module_pi_init(void)
{
        unsigned int i;

        minit_pi_low_init();
        minit_pi_hw_clear();
        pi_handler_total = 0U;
        pi_enabled_mask = 0U;
        for (i = 0U; i <= PDP10_PI_LEVELS; ++i)
                pi_level_count[i] = 0U;
        for (i = 0U; i < PDP10_PI_HANDLER_CAPACITY; ++i)
                pdp10_pi_handlers[i] = 0;
        for (i = 0U; i < PDP10_PI_LEVELS; ++i)
                pdp10_pi_level_span[i] = 0;
}

int
module_pi_register(unsigned int level, unsigned int handler)
{
        unsigned int start;
        unsigned int count;
        unsigned int insert;
        unsigned int i;

        if (level < PDP10_PI_LEVEL_MIN || level > PDP10_PI_LEVEL_MAX ||
            handler == 0U || pi_handler_total >= PDP10_PI_HANDLER_CAPACITY)
                return -1;
        start = 0U;
        for (i = PDP10_PI_LEVEL_MIN; i < level; ++i)
                start += pi_level_count[i];
        count = pi_level_count[level];
        for (i = start; i < start + count; ++i) {
                if ((unsigned int)pdp10_pi_handlers[i] == handler)
                        return -1;
        }
        insert = start + count;
        for (i = pi_handler_total; i > insert; --i)
                pdp10_pi_handlers[i] = pdp10_pi_handlers[i - 1U];
        pdp10_pi_handlers[insert] = (kword_t)handler;
        ++pi_level_count[level];
        ++pi_handler_total;
        minit_pi_reindex();
        return 0;
}

int
module_pi_unregister(unsigned int level, unsigned int handler)
{
        unsigned int start;
        unsigned int count;
        unsigned int found;
        unsigned int i;

        if (level < PDP10_PI_LEVEL_MIN || level > PDP10_PI_LEVEL_MAX ||
            handler == 0U)
                return -1;
        start = 0U;
        for (i = PDP10_PI_LEVEL_MIN; i < level; ++i)
                start += pi_level_count[i];
        count = pi_level_count[level];
        found = pi_handler_total;
        for (i = start; i < start + count; ++i) {
                if ((unsigned int)pdp10_pi_handlers[i] == handler) {
                        found = i;
                        break;
                }
        }
        if (found == pi_handler_total)
                return -1;
        for (i = found; i + 1U < pi_handler_total; ++i)
                pdp10_pi_handlers[i] = pdp10_pi_handlers[i + 1U];
        --pi_handler_total;
        pdp10_pi_handlers[pi_handler_total] = 0;
        --pi_level_count[level];
        minit_pi_reindex();
        return 0;
}

static void
minit_pi_enable(unsigned int level)
{
        pi_enabled_mask |= PDP10_PI_MASK(level);
        minit_pi_hw_set((kword_t)pi_enabled_mask);
}

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
        if (diag_put6_addr == 0U) {
                kinit_put6_spaces(words);
                return 0;
        }
        while (words != 0U) {
                if (kinit_call18_1(diag_put6_addr, 0) != 0)
                        return -1;
                --words;
        }
        return 0;
}

static int
minit_newline(void)
{
        if (diag_putchar_addr == 0U) {
                kinit_newline();
                return 0;
        }
        if (kinit_call18_1(diag_putchar_addr, 015) != 0)
                return -1;
        return (int)kinit_call18_1(diag_putchar_addr, 012);
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
minit_diag_loaded(kword_t name)
{
        minit_diag_status6(name, (kword_t)SIXBIT("    LO"),
            (kword_t)SIXBIT("ADED  "));
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
        if (module_pi_register(level, handler) != 0)
                minit_fatal(name);
        minit_pi_enable(level);
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
        diag_putchar_addr = minit_export(name, base, CTY_X_PUTCHAR);
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
        module_service_set(MODULE_SERVICE_CLK_TICKS,
            minit_export(name, base, CLK_X_TICKS));
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
        module_service_set(MODULE_SERVICE_PTR_GETCHAR,
            minit_export(name, base, PTR_X_GETCHAR));
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
dcs_minit(void)
{
        kword_t name;
        kword_t st;
        unsigned int base;

        name = (kword_t)SIXBIT("DCS   ");
        minit_dcs_cono((kword_t)DCS_NATIVE_PI_LEVEL);
        st = minit_dcs_coni();
        minit_dcs_cono(0);
        if ((st & DCS_PI_MASK) != DCS_NATIVE_PI_LEVEL) {
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        minit_register(name, DCS_NATIVE_PI_LEVEL,
            minit_export(name, base, DCS_X_HANDLER));
        module_service_set(MODULE_SERVICE_DCS_GETCHAR,
            minit_export(name, base, DCS_X_GETCHAR));
        module_service_set(MODULE_SERVICE_DCS_PUTCHAR,
            minit_export(name, base, DCS_X_PUTCHAR));
        minit_dcs_cono(0);
        minit_diag_ok(name);
}

void
tty_minit(void)
{
        kword_t name;
        unsigned int base;
        unsigned int cty_putchar;
        unsigned int dcs_putchar;
        unsigned int address;

        name = (kword_t)SIXBIT("TTY   ");
        cty_putchar = diag_putchar_addr;
        dcs_putchar = module_service_get(MODULE_SERVICE_DCS_PUTCHAR);
        if (cty_putchar == 0U && dcs_putchar == 0U) {
                minit_diag_nodev(name);
                return;
        }

        base = minit_install(name);
        address = minit_export(name, base, TTY_X_CTY_PUTCHAR_ADDR);
        *(kword_t *)(unsigned long)address = (kword_t)cty_putchar;
        address = minit_export(name, base, TTY_X_DCS_PUTCHAR_ADDR);
        *(kword_t *)(unsigned long)address = (kword_t)dcs_putchar;
        module_service_set(MODULE_SERVICE_TTY_PUTCHAR,
            minit_export(name, base, TTY_X_PUTCHAR));
        minit_diag_loaded(name);
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
        minit_diag_loaded(name);
}

void
ocnsls_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("OCNSLS");
        base = minit_install(name);
        module_service_set(MODULE_SERVICE_OCNSLS_READ,
            minit_export(name, base, OCNSLS_X_READ));
        minit_diag_loaded(name);
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
