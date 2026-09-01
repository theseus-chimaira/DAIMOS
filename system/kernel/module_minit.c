#include "kinit.h"
#include "module.h"
#include "mres.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "pt.h"
#include "card.h"
#include "dcs.h"
#include "ge.h"
#include "dpy.h"
#include "tty.h"
#include "wcnsls.h"
#include "ocnsls.h"
#include "storage.h"
#include "slv.h"

#define CTY_X_HANDLER           0U
#define CTY_X_PUT6              1U
#define CTY_X_PUTCHAR           2U
#define CTY_X_GETCHAR           3U

#define CLK_X_HANDLER           0U
#define CLK_X_TICKS             1U
#define CLK_X_PI_SERVICE        2U
#define IO7_X_HANDLER            0U
#define IO7_X_PTR_GETCHAR        1U
#define IO7_X_PTP_PUTCHAR        2U
#define IO7_X_CR_READ_CARD       3U
#define IO7_X_CP_PUNCH_CARD      4U
#define DCS_X_HANDLER           0U
#define DCS_X_GETCHAR           1U
#define DCS_X_PUTCHAR           2U
#define GE_X_HANDLER            0U
#define GE_X_GETCHAR            1U
#define GE_X_PUTCHAR            2U
#define DPY_X_HANDLER           0U
#define DPY_X_PUTWORD           1U
#define DPY_X_CLK_PI_SERVICE_CALL 2U
#define TTY_X_PUTCHAR           0U
#define TTY_X_CTY_PUTCHAR_ADDR  1U
#define TTY_X_DCS_PUTCHAR_ADDR  2U
#define TTY_X_GE_PUTCHAR_ADDR   3U
#define WCNSLS_X_READ           0U
#define OCNSLS_X_READ           0U
#define STORAGE_X_HANDLER        0U
#define STORAGE_X_DTC_READ_BLOCK 1U
#define STORAGE_X_MTC_READ_WORDS 2U
#define STORAGE_X_DSK_READ_SECTOR 3U
#define STORAGE_X_DSK_WRITE_SECTOR 4U
#define STORAGE_X_MTC_WRITE_WORDS  5U
#define STORAGE_X_DTC_WRITE_BLOCK   6U
#define STORAGE_X_DCT_HANDLER       7U

#define SLV_PI_MASK             0000007UL
#define SLV_CO_CLEAR_IRQ        0000010UL
#define SLV_PROBE_PI            7U

static unsigned int diag_put6_addr;
static unsigned int diag_putchar_addr;
static unsigned int clk_pi_handler_addr;
static unsigned int clk_pi_service_addr;
static unsigned int io7_mres_base;
static unsigned int io7_pi_handler_addr;
static unsigned int storage_mres_base;
static unsigned int storage_pi_handler_addr;
static unsigned int storage_dct_handler_addr;


static unsigned int pi_level_count[PDP10_PI_LEVELS + 1U];
static unsigned int pi_handler_total;
static unsigned int pi_enabled_mask;

static volatile kword_t *
minit_pi_span_slot(unsigned int level)
{
        if (level == 1U)
                return (volatile kword_t *)(unsigned long)000037U;
        if (level == 2U)
                return (volatile kword_t *)(unsigned long)000040U;
        if (level == 3U)
                return (volatile kword_t *)(unsigned long)000041U;
        return &pdp10_pi_level_span[level - 4U];
}

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
                *minit_pi_span_slot(level) =
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
        for (i = PDP10_PI_LEVEL_MIN; i <= PDP10_PI_LEVEL_MAX; ++i)
                *minit_pi_span_slot(i) = 0;
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
        module_service_set(MODULE_SERVICE_CTY_PUTCHAR, diag_putchar_addr);
        module_service_set(MODULE_SERVICE_CTY_GETCHAR,
            minit_export(name, base, CTY_X_GETCHAR));
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
        clk_pi_handler_addr = minit_export(name, base, CLK_X_HANDLER);
        clk_pi_service_addr = minit_export(name, base, CLK_X_PI_SERVICE);
        minit_register(name, CLK_NATIVE_PI_LEVEL, clk_pi_handler_addr);
        module_service_set(MODULE_SERVICE_CLK_TICKS,
            minit_export(name, base, CLK_X_TICKS));
        minit_clk_cono((kword_t)CLK_NATIVE_PI_LEVEL | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        minit_diag_hz();
}

static unsigned int
io7_install(kword_t name)
{
        if (io7_mres_base == 0U) {
                io7_mres_base = minit_install(name);
                io7_pi_handler_addr = minit_export(name, io7_mres_base,
                    IO7_X_HANDLER);
                minit_register(name, PT_NATIVE_PI_LEVEL, io7_pi_handler_addr);
        }
        return io7_mres_base;
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
        base = io7_install(name);
        module_service_set(MODULE_SERVICE_PTR_GETCHAR,
            minit_export(name, base, IO7_X_PTR_GETCHAR));
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
        base = io7_install(name);
        minit_ptp_cono(PT_NATIVE_PI_LEVEL);
        module_service_set(MODULE_SERVICE_PTP_PUTCHAR,
            minit_export(name, base, IO7_X_PTP_PUTCHAR));
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
        base = io7_install(name);
        minit_cr_cono((kword_t)CARD_NATIVE_PI_LEVEL | CR_CO_CLR_DRDY |
            CR_CO_CLR_END_CARD | CR_CO_CLR_DATA_MISS);
        module_service_set(MODULE_SERVICE_CR_READ_CARD,
            minit_export(name, base, IO7_X_CR_READ_CARD));
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
        base = io7_install(name);
        minit_cp_cono((kword_t)CARD_NATIVE_PI_LEVEL | CP_CO_CLR_PUNCH);
        module_service_set(MODULE_SERVICE_CP_PUNCH_CARD,
            minit_export(name, base, IO7_X_CP_PUNCH_CARD));
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
ge_minit(void)
{
        kword_t name;
        kword_t ist;
        kword_t ost;
        unsigned int base;

        name = (kword_t)SIXBIT("GE    ");
        minit_gtyi_cono((kword_t)GE_NATIVE_PI_LEVEL);
        ist = minit_gtyi_coni();
        minit_gtyi_cono(0);
        minit_gtyo_cono((kword_t)GE_NATIVE_PI_LEVEL);
        ost = minit_gtyo_coni();
        minit_gtyo_cono(0);
        if ((ist & GTYI_PI_MASK) != GE_NATIVE_PI_LEVEL &&
            (ost & GTYO_PI_MASK) != GE_NATIVE_PI_LEVEL) {
                minit_diag_nodev(name);
                return;
        }
        if ((ist & GTYI_PI_MASK) != GE_NATIVE_PI_LEVEL ||
            (ost & GTYO_PI_MASK) != GE_NATIVE_PI_LEVEL) {
                minit_diag_notok(name);
                return;
        }

        base = minit_install(name);
        minit_register(name, GE_NATIVE_PI_LEVEL,
            minit_export(name, base, GE_X_HANDLER));

        module_service_set(MODULE_SERVICE_GE_GETCHAR,
            minit_export(name, base, GE_X_GETCHAR));
        module_service_set(MODULE_SERVICE_GE_PUTCHAR,
            minit_export(name, base, GE_X_PUTCHAR));
        minit_gtyi_cono(0);
        minit_gtyo_cono((kword_t)GTYO_CO_FROB);
        minit_diag_ok(name);
}

static unsigned int
dpy_param_mode(unsigned int mode)
{
        return (mode & 07U) << 13;
}

static unsigned int
dpy_point_coord(unsigned int yflag, unsigned int coord,
    unsigned int next_mode)
{
        unsigned int inst;

        inst = (next_mode & 07U) << 13;
        if (yflag != 0U)
                inst |= 0200000U;
        inst |= coord & 01777U;
        return inst & 0777777U;
}

static unsigned int
dpy_char3(unsigned int c0, unsigned int c1, unsigned int c2)
{
        return ((c0 & 077U) << 12) | ((c1 & 077U) << 6) | (c2 & 077U);
}

static kword_t
dpy_inst(unsigned int left, unsigned int right)
{
        return ((((kword_t)left) & 0777777UL) << 18) |
            (((kword_t)right) & 0777777UL);
}

static void
minit_dpy_word(kword_t name, unsigned int putword, kword_t word)
{
        if (kinit_call18_1(putword, word) != DPY_E_OK)
                minit_fatal(name);
}

static void
minit_dpy_flush_codes(kword_t name, unsigned int putword,
    unsigned int *codes, unsigned int *count, unsigned int *char_mode)
{
        unsigned int need;

        need = *char_mode == 0U ? 3U : 6U;
        while (*count < need)
                codes[(*count)++] = DPY_T342_SPACE;
        if (*char_mode == 0U) {
                minit_dpy_word(name, putword, dpy_inst(
                    dpy_param_mode(DPY_MODE_CHAR),
                    dpy_char3(codes[0], codes[1], codes[2])));
                *char_mode = 1U;
        } else {
                minit_dpy_word(name, putword, dpy_inst(
                    dpy_char3(codes[0], codes[1], codes[2]),
                    dpy_char3(codes[3], codes[4], codes[5])));
        }
        *count = 0U;
}

static void
minit_dpy_code(kword_t name, unsigned int putword, unsigned int code,
    unsigned int *codes, unsigned int *count, unsigned int *char_mode)
{
        unsigned int need;

        codes[(*count)++] = code & 077U;
        need = *char_mode == 0U ? 3U : 6U;
        if (*count == need)
                minit_dpy_flush_codes(name, putword, codes, count, char_mode);
}

/* The Type 342 upper-case set uses 1..32 for A..Z and ASCII 040..077
 * directly for the punctuation/digits needed by the boot version banner. */
static void
minit_dpy_char(kword_t name, unsigned int putword, unsigned int ch,
    unsigned int *codes, unsigned int *count, unsigned int *char_mode)
{
        unsigned int code;

        if (ch >= 'A' && ch <= 'Z')
                code = ch - 'A' + 1U;
        else if (ch >= 040U && ch <= 077U)
                code = ch;
        else
                code = 077U;
        minit_dpy_code(name, putword, code, codes, count, char_mode);
}

static void
minit_dpy_banner(kword_t name, unsigned int putword)
{
        static const char title[] = "DAIMOS ";
        static const char version[] = DAIMON_VERSION_TEXT;
        unsigned int codes[6];
        unsigned int count;
        unsigned int char_mode;
        unsigned int i;

        count = 0U;
        char_mode = 0U;

        minit_dpy_word(name, putword, dpy_inst(dpy_param_mode(DPY_MODE_POINT),
            dpy_point_coord(0U, 0240U, DPY_MODE_POINT)));
        minit_dpy_word(name, putword, dpy_inst(
            dpy_point_coord(1U, 01000U, DPY_MODE_PARAM),
            dpy_param_mode(DPY_MODE_PARAM)));

        for (i = 0U; title[i] != '\0'; ++i)
                minit_dpy_char(name, putword, (unsigned int)title[i],
                    codes, &count, &char_mode);
        for (i = 0U; version[i] != '\0'; ++i)
                minit_dpy_char(name, putword, (unsigned int)version[i],
                    codes, &count, &char_mode);
        if (count != 0U)
                minit_dpy_flush_codes(name, putword, codes, &count, &char_mode);
}

void
dpy_minit(void)
{
        kword_t name;
        kword_t st;
        kword_t probe;
        unsigned int base;
        unsigned int handler;
        unsigned int putword;
        unsigned int address;

        name = (kword_t)SIXBIT("DPY   ");
        probe = (kword_t)DPY_NATIVE_PI_LEVEL |
            ((kword_t)DPY_PROBE_SPEC_PI << DPY_SPEC_PI_SHIFT);
        minit_dpy_cono(probe);
        st = minit_dpy_coni();
        minit_dpy_cono(0);
        if ((st & (DPY_DATA_PI_MASK | DPY_SPEC_PI_MASK)) == 0) {
                minit_diag_nodev(name);
                return;
        }
        if ((st & DPY_DATA_PI_MASK) != DPY_NATIVE_PI_LEVEL ||
            ((st & DPY_SPEC_PI_MASK) >> DPY_SPEC_PI_SHIFT) !=
            DPY_PROBE_SPEC_PI) {
                minit_diag_notok(name);
                return;
        }

        minit_dpy_cono(DPY_CO_INIT);
        minit_dpy_cono(0);
        base = minit_install(name);
        handler = minit_export(name, base, DPY_X_HANDLER);
        putword = minit_export(name, base, DPY_X_PUTWORD);
        address = minit_export(name, base, DPY_X_CLK_PI_SERVICE_CALL);
        if (clk_pi_service_addr != 0U)
                *(kword_t *)(unsigned long)address =
                    (*(kword_t *)(unsigned long)address &
                    ~((kword_t)KINIT_HALF_MASK)) |
                    (kword_t)clk_pi_service_addr;

        /* DPY and the APR line clock share one PI6 table entry. */
        if (clk_pi_handler_addr != 0U) {
                if (module_pi_unregister(CLK_NATIVE_PI_LEVEL,
                    clk_pi_handler_addr) != 0 ||
                    module_pi_register(DPY_NATIVE_PI_LEVEL, handler) != 0)
                        minit_fatal(name);
        } else {
                minit_register(name, DPY_NATIVE_PI_LEVEL, handler);
        }

        module_service_set(MODULE_SERVICE_DPY_PUTWORD, putword);
        minit_dpy_cono((kword_t)DPY_NATIVE_PI_LEVEL);
        minit_dpy_banner(name, putword);
        minit_diag_ok(name);
}

void
tty_minit(void)
{
        kword_t name;
        unsigned int base;
        unsigned int cty_putchar;
        unsigned int dcs_putchar;
        unsigned int ge_putchar;
        unsigned int address;

        name = (kword_t)SIXBIT("TTY   ");
        cty_putchar = diag_putchar_addr;
        dcs_putchar = module_service_get(MODULE_SERVICE_DCS_PUTCHAR);
        ge_putchar = module_service_get(MODULE_SERVICE_GE_PUTCHAR);
        if (cty_putchar == 0U && dcs_putchar == 0U && ge_putchar == 0U) {
                minit_diag_nodev(name);
                return;
        }

        base = minit_install(name);
        address = minit_export(name, base, TTY_X_CTY_PUTCHAR_ADDR);
        if (cty_putchar != 0U)
                *(kword_t *)(unsigned long)address =
                    (*(kword_t *)(unsigned long)address &
                    ~((kword_t)KINIT_HALF_MASK)) | (kword_t)cty_putchar;
        address = minit_export(name, base, TTY_X_DCS_PUTCHAR_ADDR);
        if (dcs_putchar != 0U)
                *(kword_t *)(unsigned long)address =
                    (*(kword_t *)(unsigned long)address &
                    ~((kword_t)KINIT_HALF_MASK)) | (kword_t)dcs_putchar;
        address = minit_export(name, base, TTY_X_GE_PUTCHAR_ADDR);
        if (ge_putchar != 0U)
                *(kword_t *)(unsigned long)address =
                    (*(kword_t *)(unsigned long)address &
                    ~((kword_t)KINIT_HALF_MASK)) | (kword_t)ge_putchar;
        module_service_set(MODULE_SERVICE_TTY_PUTCHAR,
            minit_export(name, base, TTY_X_PUTCHAR));
        minit_diag_loaded(name);
}

static const kword_t minit_wcnsls_title[] = {
        0364306143076UL, 0164307743061UL, 0371020410237UL,
        0216726543061UL, 0164306143056UL, 0174101602076UL, 0
};

static const kword_t minit_wcnsls_digits[] = {
        0164316563056UL, 0043020410216UL, 0164204210437UL,
        0360205602076UL, 0021452276102UL, 0374103602076UL,
        0164103643056UL, 0370210420410UL, 0164305643056UL,
        0164305702056UL
};

static kword_t
minit_wcnsls_version_glyph(unsigned int ch)
{
        if (ch >= '0' && ch <= '9')
                return minit_wcnsls_digits[ch - '0'];
        if (ch == 'V')
                return 0214306142504UL;
        if (ch == '.')
                return 0000000000306UL;
        if (ch == '-')
                return 0000003700000UL;
        return 0;
}

static void
minit_wcnsls_glyph(kword_t glyph, unsigned int x)
{
        unsigned int row;
        unsigned int col;
        unsigned int bit;

        for (row = 0U; row < 7U; ++row) {
                for (col = 0U; col < 5U; ++col) {
                        bit = 34U - row * 5U - col;
                        if (((glyph >> bit) & 1UL) != 0)
                                minit_wcnsls_plot(WCNSLS_COORD(
                                    x + col * 7U, 0330U - row * 7U));
                }
        }
}

static void
minit_wcnsls_banner(void)
{
        static const char version[] = DAIMON_VERSION_TEXT;
        unsigned int x;
        unsigned int i;

        minit_wcnsls_cono(WCNSLS_CO_SPACEWAR | WCNSLS_CO_GREEN_FULL);
        x = 025U;
        for (i = 0U; i < sizeof(minit_wcnsls_title) /
            sizeof(minit_wcnsls_title[0]); ++i) {
                minit_wcnsls_glyph(minit_wcnsls_title[i], x);
                x += 42U;
        }
        for (i = 0U; version[i] != '\0'; ++i) {
                minit_wcnsls_glyph(
                    minit_wcnsls_version_glyph((unsigned int)version[i]), x);
                x += 42U;
        }
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
        minit_wcnsls_banner();
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

static unsigned int
storage_install(kword_t name)
{
        if (storage_mres_base == 0U) {
                storage_mres_base = minit_install(name);
                storage_pi_handler_addr = minit_export(name, storage_mres_base,
                    STORAGE_X_HANDLER);
                storage_dct_handler_addr = minit_export(name, storage_mres_base,
                    STORAGE_X_DCT_HANDLER);
                minit_register(name, STORAGE_NATIVE_PI_LEVEL,
                    storage_pi_handler_addr);
                minit_register(name, STORAGE_DCT_PI_LEVEL,
                    storage_dct_handler_addr);
        }
        return storage_mres_base;
}

void
storage_minit(unsigned int kind, kword_t name)
{
        kword_t st;
        unsigned int base;

        st = minit_storage_probe(kind);
        if ((st & STORAGE_ST_PI_MASK) != STORAGE_NATIVE_PI_LEVEL) {
                minit_diag_nodev(name);
                return;
        }
        base = storage_install(name);
        module_service_set(MODULE_SERVICE_DTC_READ_BLOCK + kind,
            minit_export(name, base, STORAGE_X_DTC_READ_BLOCK + kind));
        module_service_set(MODULE_SERVICE_DTC_WRITE_BLOCK - kind,
            minit_export(name, base, STORAGE_X_DTC_WRITE_BLOCK - kind));
        minit_diag_ok(name);
}

void
slv_minit(void)
{
        kword_t st;
        kword_t name;

        name = (kword_t)SIXBIT("SLV   ");
        minit_slv_cono(SLV_PROBE_PI);
        st = minit_slv_coni();
        minit_slv_cono(SLV_CO_CLEAR_IRQ | SLV_NATIVE_PI_LEVEL);
        if ((st & SLV_PI_MASK) != SLV_PROBE_PI) {
                minit_diag_nodev(name);
                return;
        }
        {
                unsigned int base;
                unsigned int handler;

                base = minit_install(name);
                handler = minit_export(name, base, 0U);
                minit_register(name, SLV_NATIVE_PI_LEVEL, handler);
        }
        minit_diag_ok(name);
}
