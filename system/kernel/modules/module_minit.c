/**
 * @file module_minit.c
 * @brief Transient KINIT probe/install logic for built-in DAIMOS modules.
 *
 * MINIT functions run in module_table.s order. Each probes its PDP-6 device or
 * prerequisite service, installs the associated MRES package when usable,
 * patches resident call/jump sites, registers PI handlers, and publishes boot
 * services for later MINIT consumers. This file is reclaimed with KINIT; only
 * the installed MRES images and patched KCORE words remain resident.
 *
 * Hardware probes deliberately avoid resident-driver dependencies because they
 * execute before those drivers exist. Fatal installation/ABI failures halt the
 * boot; absent optional hardware is normally omitted from the boot log.
 */
#include "kinit.h"
#include "module.h"
#include "mres.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "pt.h"
#include "lpt.h"
#include "card.h"
#include "dcs.h"
#include "ge.h"
#include "dpy.h"
#include "dsk270.h"
#include "drm236.h"
#include "tty.h"
#include "wcnsls.h"
#include "ocnsls.h"
#include "storage.h"
#include "slv.h"
#include "fs_mres.h"
#include "d6fs.h"
#include "dtfs.h"
#include "blockset_mres.h"
#include "blockset_boot.h"
#include "auxstore.h"
#include "logstore.h"
#if KINIT_FULL
#include "root_select.h"
#endif
#include "badmap.h"
#include "mm.h"
#include "monitorfs.h"

#define CTY_X_HANDLER           0U
#define CTY_X_PUTCHAR           1U
#define CTY_X_GETCHAR           2U

#define CLK_X_HANDLER           0U
#define CLK_X_TICKS             1U
#define CLK_X_PI_SERVICE        2U
#define PTR_X_HANDLER            0U
#define PTR_X_GETCHAR            1U
#define PTR_X_READ_WORDS         2U
#define PTP_X_HANDLER            0U
#define PTP_X_PUTCHAR            1U
#define PTP_X_WRITE_WORDS        2U
#define CR_X_HANDLER             0U
#define CR_X_READ_CARD           1U
#define CR_X_READ_WORDS          2U
#define CP_X_HANDLER             0U
#define CP_X_PUNCH_CARD          1U
#define CP_X_WRITE_WORDS         2U
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
#define TTY_X_GETCHAR           1U
#define TTY_X_CTY_PUTCHAR_ADDR  2U
#define TTY_X_DCS_PUTCHAR_ADDR  3U
#define TTY_X_GE_PUTCHAR_ADDR   4U
#define TTY_X_CTY_GETCHAR_ADDR  5U
#define TTY_X_DCS_GETCHAR_ADDR  6U
#define TTY_X_GE_GETCHAR_ADDR   7U
#define TTY_X_WRITE_S6REC       8U
#define TTY_X_READ_S6REC        9U
#define WCNSLS_X_READ           0U
#define OCNSLS_X_READ           0U
#define TAPE_X_HANDLER           0U
#define TAPE_X_DTC_READ_BLOCK    1U
#define TAPE_X_MTC_SERVICE       2U
#define TAPE_X_DTC_WRITE_BLOCK   3U
#define TAPE_X_DCT_HANDLER       4U
#define DSK_X_HANDLER            0U
#define DSK_X_READ_SECTOR        1U
#define DSK_X_WRITE_SECTOR       2U
#define DSK_X_DCT_HANDLER        3U
#define DSK_X_WATCHDOG           4U
#define DRM_X_HANDLER             0U
#define DRM_X_READ_BLOCK          1U
#define DRM_X_WRITE_BLOCK         2U

#define LOGSTORE_X_DISPATCH       0U
#define LOGSTORE_X_STATE          1U
#define LOGSTORE_X_READ_JUMP      2U
#define LOGSTORE_X_WRITE_JUMP     3U


#define DRM_PROBE_PI              7U
#define DRM_PI_MASK               0000007UL
#define DRM_NATIVE_PI_LEVEL       2U

static unsigned int diag_putchar_addr;
static unsigned int clk_pi_handler_addr;
static unsigned int clk_pi_service_addr;
static unsigned int tape_mres_base;
static unsigned int dsk_mres_base;
static unsigned int storage_router_registered;
static kword_t *dtfs_runtime_dir_ptr;
unsigned int blockset_read_addr;
unsigned int blockset_write_addr;
unsigned int blockset_state_addr;
unsigned int d6fs_backing_read_addr;
unsigned int d6fs_backing_write_addr;
#if KINIT_BADMAP
static kword_t *badmap_runtime_state;
#endif

extern kword_t storage_pi_handler;
extern kword_t storage_dct_handler;
extern kword_t storage_pi_dsk_jump;
extern kword_t storage_pi_tape_jump;
extern kword_t storage_dct_dsk_jump;
extern kword_t storage_dct_tape_jump;
extern kword_t storage_clock_dsk_jump;

extern kword_t dsk270_read_jump;
extern kword_t dsk270_write_jump;
extern kword_t drm236_read_jump;
extern kword_t drm236_write_jump;
extern kword_t native_sys_getchar_call;
extern kword_t sys_dtc_read_block_jump;
extern kword_t sys_dtc_write_block_jump;
extern kword_t sys_logstore_service_jump;
extern kword_t sys_mtc_service_jump;
extern kword_t native_sys_putchar_call;
extern kword_t tty_write_s6rec_jump;
extern kword_t tty_read_s6rec_jump;
extern kword_t ptr_read_words_jump;
extern kword_t ptp_write_words_jump;
extern kword_t cr_read_words_jump;
extern kword_t cp_write_words_jump;
extern int d6fs_reader_bootstrap_call(kword_t backing_ops);


static unsigned int pi_level_count[PDP10_PI_LEVELS + 1U];

static void storage_patch_jump(kword_t *word, unsigned int address);
static void storage_patch_module_jump(unsigned int base, kword_t *word,
    unsigned int address);
static unsigned int pi_handler_total;
static unsigned int pi_enabled_mask;

static volatile kword_t *
minit_pi_span_slot(unsigned int level)
{
        return &pdp10_pi_level_span[level - 1U];
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

/** Rebuild compact PI handler spans and direct/single-handler dispatch jumps. */
static void
minit_pi_reindex(void)
{
        unsigned int level;
        unsigned int start;

        start = 0U;
        for (level = PDP10_PI_LEVEL_MIN; level <= PDP10_PI_LEVEL_MAX;
            ++level) {
                unsigned int count;
                kword_t *jump;
                unsigned int target;

                count = pi_level_count[level];
                *minit_pi_span_slot(level) = minit_pi_span(start, count);
                if (level <= 6U) {
                        if (level == 1U)
                                jump = &pdp10_pi_level1_dispatch_jump;
                        else if (level == 2U)
                                jump = &pdp10_pi_level2_dispatch_jump;
                        else if (level == 3U)
                                jump = &pdp10_pi_level3_dispatch_jump;
                        else if (level == 4U)
                                jump = &pdp10_pi_level4_dispatch_jump;
                        else if (level == 5U)
                                jump = &pdp10_pi_level5_dispatch_jump;
                        else
                                jump = &pdp10_pi_level6_dispatch_jump;
                        target = (unsigned int)(unsigned long)&pdp10_pi_dispatch;
                        if (count == 1U)
                                target = (unsigned int)pdp10_pi_handlers[start];
                        storage_patch_jump(jump, target);
                }
                start += count;
        }
}

/** Initialize PDP-6 low-core PI vectors and empty transient handler tables. */
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

/** Register one handler while preserving handlers grouped by PI level. */
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

/** Unregister one handler and compact/reindex the PI dispatch table. */
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
        unsigned int i;
        unsigned int ch;

        if (diag_putchar_addr == 0U) {
                kinit_put6(word);
                return 0;
        }
        for (i = 0U; i < 6U; ++i) {
                ch = (unsigned int)((word >> 30) & 077UL) + 040U;
                if (kinit_call18_1(diag_putchar_addr, (kword_t)ch) != 0)
                        return -1;
                word <<= 6;
        }
        return 0;
}

static int
minit_spaces(unsigned int words)
{
        if (diag_putchar_addr == 0U) {
                kinit_put6_spaces(words);
                return 0;
        }
        while (words != 0U) {
                if (minit_put6(0) != 0)
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
        /* Non-detectable hardware is omitted from the boot log. */
        (void)name;
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

/** Install the current MINIT's MRES package or halt with its device name. */
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

/** Resolve one export from the current relocated MRES package or halt. */
static unsigned int
minit_export(kword_t name, unsigned int base, unsigned int index)
{
        unsigned int address;

        address = mres_export(module_current_mres(), base, index);
        if (address == 0U)
                minit_fatal(name);
        return address;
}

/** Register a resident PI handler and enable its hardware PI level. */
static void
minit_register(kword_t name, unsigned int level, unsigned int handler)
{
        if (module_pi_register(level, handler) != 0)
                minit_fatal(name);
        minit_pi_enable(level);
}

/** @brief Probe CTY, install its resident driver, and publish console I/O. */
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
        diag_putchar_addr = minit_export(name, base, CTY_X_PUTCHAR);
        module_service_set(MODULE_SERVICE_CTY_PUTCHAR, diag_putchar_addr);
        storage_patch_jump(&native_sys_putchar_call, diag_putchar_addr);
        base = minit_export(name, base, CTY_X_GETCHAR);
        module_service_set(MODULE_SERVICE_CTY_GETCHAR, base);
        storage_patch_jump(&native_sys_getchar_call, base);
        minit_diag_ok(name);
}

/** @brief Probe the APR line clock and install/publish the resident clock service. */
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

#if KINIT_FULL
/** @brief Probe the paper-tape reader and install its resident input service. */
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
        storage_patch_jump(&ptr_read_words_jump,
            minit_export(name, base, PTR_X_READ_WORDS));
        minit_ptr_cono(PT_NATIVE_PI_LEVEL);
        minit_diag_ok(name);
}

/** @brief Probe the paper-tape punch and install its resident output service. */
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
        storage_patch_jump(&ptp_write_words_jump,
            minit_export(name, base, PTP_X_WRITE_WORDS));
        minit_diag_ok(name);
}

/** @brief Probe the synchronous line printer and publish its resident entry. */
void
lpt_minit(void)
{
        kword_t name;
        kword_t st;
        name = (kword_t)SIXBIT("LPT   ");
        /* No PI level is needed by the small synchronous driver.  DONE is
         * software-settable on the PDP-6/SIMH interface and primes DATAO. */
        minit_lpt_cono(LPT_ST_DONE);
        st = minit_lpt_coni();
        if ((st & LPT_ST_DONE) == 0UL) {
                minit_lpt_cono(0);
                minit_diag_nodev(name);
                return;
        }
        module_service_set(MODULE_SERVICE_LPT_PUTCHAR,
            (unsigned int)(unsigned long)&lpt_putchar);
        minit_diag_ok(name);
}

/** @brief Probe the card reader, install its MRES, and register its PI handler. */
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
        storage_patch_jump(&cr_read_words_jump,
            minit_export(name, base, CR_X_READ_WORDS));
        minit_diag_ok(name);
}

/** @brief Probe the card punch, install its MRES, and register its PI handler. */
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
        storage_patch_jump(&cp_write_words_jump,
            minit_export(name, base, CP_X_WRITE_WORDS));
        minit_diag_ok(name);
}


#endif

/** @brief Probe DCS terminal hardware and publish resident character I/O. */
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

/** @brief Probe GE terminal input/output interfaces and publish resident I/O. */
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

extern kword_t minit_dpy_banner_words[];
extern kword_t minit_dpy_banner_words_end[];

#if KINIT_FULL
static void
minit_dpy_word(kword_t name, unsigned int putword, kword_t word)
{
        if (kinit_call18_1(putword, word) != DPY_E_OK)
                minit_fatal(name);
}

static void
minit_dpy_banner(kword_t name, unsigned int putword)
{
        kword_t *word;

        for (word = minit_dpy_banner_words;
            word != minit_dpy_banner_words_end; ++word)
                minit_dpy_word(name, putword, *word);
}

/** @brief Probe the display, install its combined clock/display PI service, and banner. */
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
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, clk_pi_service_addr);

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

#endif

/** @brief Bind the TTY multiplexer to whichever CTY/DCS/GE services were installed. */
void
tty_minit(void)
{
        kword_t name;
        unsigned int base;
        unsigned int cty_putchar;
        unsigned int dcs_putchar;
        unsigned int ge_putchar;
        unsigned int cty_getchar;
        unsigned int dcs_getchar;
        unsigned int ge_getchar;
        unsigned int address;
        unsigned int service;

        name = (kword_t)SIXBIT("TTY   ");
        cty_putchar = diag_putchar_addr;
        dcs_putchar = module_service_get(MODULE_SERVICE_DCS_PUTCHAR);
        ge_putchar = module_service_get(MODULE_SERVICE_GE_PUTCHAR);
        cty_getchar = module_service_get(MODULE_SERVICE_CTY_GETCHAR);
        dcs_getchar = module_service_get(MODULE_SERVICE_DCS_GETCHAR);
        ge_getchar = module_service_get(MODULE_SERVICE_GE_GETCHAR);
        if (cty_putchar == 0U && dcs_putchar == 0U && ge_putchar == 0U &&
            cty_getchar == 0U && dcs_getchar == 0U && ge_getchar == 0U) {
                minit_diag_nodev(name);
                return;
        }

        base = minit_install(name);
        address = minit_export(name, base, TTY_X_CTY_PUTCHAR_ADDR);
        if (cty_putchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, cty_putchar);
        address = minit_export(name, base, TTY_X_DCS_PUTCHAR_ADDR);
        if (dcs_putchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, dcs_putchar);
        address = minit_export(name, base, TTY_X_GE_PUTCHAR_ADDR);
        if (ge_putchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, ge_putchar);
        address = minit_export(name, base, TTY_X_CTY_GETCHAR_ADDR);
        if (cty_getchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, cty_getchar);
        address = minit_export(name, base, TTY_X_DCS_GETCHAR_ADDR);
        if (dcs_getchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, dcs_getchar);
        address = minit_export(name, base, TTY_X_GE_GETCHAR_ADDR);
        if (ge_getchar != 0U)
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)address, ge_getchar);

        service = minit_export(name, base, TTY_X_PUTCHAR);
        module_service_set(MODULE_SERVICE_TTY_PUTCHAR, service);
        storage_patch_jump(&native_sys_putchar_call, service);
        service = minit_export(name, base, TTY_X_GETCHAR);
        storage_patch_jump(&native_sys_getchar_call, service);
        service = minit_export(name, base, TTY_X_WRITE_S6REC);
        storage_patch_jump(&tty_write_s6rec_jump, service);
        service = minit_export(name, base, TTY_X_READ_S6REC);
        storage_patch_jump(&tty_read_s6rec_jump, service);
        minit_diag_loaded(name);
}

extern kword_t minit_wcnsls_banner_glyphs[];
extern kword_t minit_wcnsls_banner_glyphs_end[];

#if KINIT_FULL
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
        kword_t *glyph;
        unsigned int x;

        minit_wcnsls_cono(WCNSLS_CO_SPACEWAR | WCNSLS_CO_GREEN_FULL);
        x = 025U;
        for (glyph = minit_wcnsls_banner_glyphs;
            glyph != minit_wcnsls_banner_glyphs_end; ++glyph) {
                minit_wcnsls_glyph(*glyph, x);
                x += 42U;
        }
}

/** @brief Probe WCNSLS, install its resident service, and emit the boot banner. */
void
wcnsls_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("WCNSLS");
        /* The real switch register is active-low in its unused bits; the
         * null-device path returns an all-zero DATAI. */
        minit_wcnsls_cono(WCNSLS_CO_SPACEWAR);
        if (minit_wcnsls_datai() == 0UL) {
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        module_service_set(MODULE_SERVICE_WCNSLS_READ,
            minit_export(name, base, WCNSLS_X_READ));
        minit_wcnsls_banner();
        minit_diag_loaded(name);
}

/** @brief Probe OCNSLS and install/publish its resident read service. */
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

#endif

static void
storage_patch_jump(kword_t *word, unsigned int address)
{
        *word = (*word & ~((kword_t)KINIT_HALF_MASK)) |
            (kword_t)(address & KINIT_HALF_MASK);
}

static void
storage_patch_module_jump(unsigned int base, kword_t *word,
    unsigned int address)
{
        (void)base;
        storage_patch_jump(word, address);
}

static void
storage_register_router(kword_t name)
{
        if (storage_router_registered != 0U)
                return;
        minit_register(name, STORAGE_NATIVE_PI_LEVEL,
            (unsigned int)(unsigned long)&storage_pi_handler);
        minit_register(name, STORAGE_DCT_PI_LEVEL,
            (unsigned int)(unsigned long)&storage_dct_handler);
        storage_router_registered = 1U;
}

static unsigned int
storage_install(unsigned int kind, kword_t name)
{
        unsigned int base;
        unsigned int pi;
        unsigned int dct;

        storage_register_router(name);
        if (kind == 2U) {
                if (dsk_mres_base != 0U)
                        return dsk_mres_base;
                base = minit_install(name);
                pi = minit_export(name, base, DSK_X_HANDLER);
                dct = minit_export(name, base, DSK_X_DCT_HANDLER);
                storage_patch_jump(&storage_pi_dsk_jump, pi);
                storage_patch_jump(&storage_dct_dsk_jump, dct);
                dsk_mres_base = base;
                return base;
        }
        if (tape_mres_base != 0U)
                return tape_mres_base;
        base = minit_install(name);
        pi = minit_export(name, base, TAPE_X_HANDLER);
        dct = minit_export(name, base, TAPE_X_DCT_HANDLER);
        storage_patch_jump(&storage_pi_tape_jump, pi);
        storage_patch_jump(&storage_dct_tape_jump, dct);
        tape_mres_base = base;
        return base;
}

/** @brief Shared Type-136 DTC/MTC/DSK probe/install path selected by kind. */
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
        base = storage_install(kind, name);
        if (kind == 0U) {
                module_service_set(MODULE_SERVICE_DTC_READ_BLOCK,
                    minit_export(name, base, TAPE_X_DTC_READ_BLOCK));
                storage_patch_jump(&sys_dtc_read_block_jump,
                    minit_export(name, base, TAPE_X_DTC_READ_BLOCK));
                module_service_set(MODULE_SERVICE_DTC_WRITE_BLOCK,
                    minit_export(name, base, TAPE_X_DTC_WRITE_BLOCK));
                storage_patch_jump(&sys_dtc_write_block_jump,
                    minit_export(name, base, TAPE_X_DTC_WRITE_BLOCK));
        } else if (kind == 1U) {
                unsigned int mtc_service;

                mtc_service = minit_export(name, base, TAPE_X_MTC_SERVICE);
                module_service_set(MODULE_SERVICE_MTC, mtc_service);
                storage_patch_jump(&sys_mtc_service_jump, mtc_service);
        } else {
                {
                        unsigned int read_service;
                        unsigned int write_service;

                        read_service = minit_export(name, base,
                            DSK_X_READ_SECTOR);
                        write_service = minit_export(name, base,
                            DSK_X_WRITE_SECTOR);
                        storage_patch_jump(&dsk270_read_jump, read_service);
                        storage_patch_jump(&dsk270_write_jump, write_service);
                        module_service_set(MODULE_SERVICE_DSK_READ_SECTOR,
                            read_service);
                        module_service_set(MODULE_SERVICE_DSK_WRITE_SECTOR,
                            write_service);
                        storage_patch_jump(&storage_clock_dsk_jump,
                            minit_export(name, base, DSK_X_WATCHDOG));
                }
        }
        minit_diag_ok(name);
}

#if KINIT_FULL
/** @brief Probe Type-167/236 drum hardware and install resident block I/O. */
void
drm236_minit(void)
{
        kword_t name;
        kword_t st;
        unsigned int base;
        unsigned int handler;
        unsigned int read_service;
        unsigned int write_service;

        name = (kword_t)SIXBIT("DRM236");
        st = minit_drm236_probe();
        if ((st & DRM_PI_MASK) != DRM_PROBE_PI) {
                minit_diag_nodev(name);
                return;
        }
        base = minit_install(name);
        handler = minit_export(name, base, DRM_X_HANDLER);
        read_service = minit_export(name, base, DRM_X_READ_BLOCK);
        write_service = minit_export(name, base, DRM_X_WRITE_BLOCK);
        minit_register(name, DRM_NATIVE_PI_LEVEL, handler);
        storage_patch_jump(&drm236_read_jump, read_service);
        storage_patch_jump(&drm236_write_jump, write_service);
        module_service_set(MODULE_SERVICE_DRM_READ_BLOCK, read_service);
        module_service_set(MODULE_SERVICE_DRM_WRITE_BLOCK, write_service);
        minit_diag_ok(name);
}


/** @brief Install the MEMFS resident service when the package is available. */
void
memfs_minit(void)
{
        kword_t name;
        unsigned int base;

        name = (kword_t)SIXBIT("MEMFS ");
        base = minit_install(name);
        {
                unsigned int service;

                service = minit_export(name, base, 0U);
                storage_patch_jump(&fs_memfs_service_jump, service);
                storage_patch_jump(&sys_memfs_usage_call,
                    minit_export(name, base, 1U));
                storage_patch_jump(&memfs_reclaim_jump,
                    minit_export(name, base, 2U));
                storage_patch_jump(&memfs_shutdown_jump,
                    minit_export(name, base, 3U));
                module_service_set(MODULE_SERVICE_MEMFS, service);
        }
        minit_diag_loaded(name);
}

/** @brief Install DTFS after DECtape block services are available. */
void
dtfs_minit(void)
{
        kword_t name;
        unsigned int base;
        unsigned int read_addr;
        unsigned int write_addr;
        unsigned int state_addr;

        name = (kword_t)SIXBIT("DTFS  ");
        read_addr = module_service_get(MODULE_SERVICE_DTC_READ_BLOCK);
        write_addr = module_service_get(MODULE_SERVICE_DTC_WRITE_BLOCK);
        if (read_addr == 0U || write_addr == 0U)
                return;
        base = minit_install(name);
        {
                unsigned int service;

                service = minit_export(name, base, 0U);
                storage_patch_jump(&fs_dtfs_service_jump, service);
                state_addr = minit_export(name, base, 1U);
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)state_addr, read_addr);
                state_addr = minit_export(name, base, 2U);
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)state_addr, write_addr);
                storage_patch_jump(&sys_dtfs_mount_jump,
                    minit_export(name, base, 3U));
                storage_patch_jump(&fs_tsfs_service_jump,
                    minit_export(name, base, 4U));
                dtfs_runtime_dir_ptr = (kword_t *)(unsigned long)
                    minit_export(name, base, 5U);
                *dtfs_runtime_dir_ptr = 0UL;
                module_service_set(MODULE_SERVICE_DTFS, service);
        }
        minit_diag_loaded(name);
}

/** @brief Finish DTFS runtime bindings after all MINITs have run. */
void
dtfs_post_minits(void)
{
        kword_t cache_base;

        if (dtfs_runtime_dir_ptr == 0)
                return;
        if (mm_alloc(0200UL, MM_TYPE_KERNEL_DYNAMIC, DTFS_CACHE_MM_OWNER,
            MM_ALLOC_LOW, &cache_base) != MM_OK)
                kinit_halt();
        *dtfs_runtime_dir_ptr = cache_base;
}

#endif

static int
root_block_services(unsigned int *readp, unsigned int *writep)
{
        if (readp == 0 || writep == 0)
                return -1;
#if KINIT_FULL
        if (root_select_class() == KINIT_ROOT_DRM) {
                if (module_service_get(MODULE_SERVICE_DRM_READ_BLOCK) == 0U ||
                    module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK) == 0U)
                        return -1;
                *readp = (unsigned int)(unsigned long)&drm236_read_block;
                *writep = (unsigned int)(unsigned long)&drm236_write_block;
                return 0;
        }
#endif
        if (module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) == 0U ||
            module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) == 0U)
                return -1;
        *readp = (unsigned int)(unsigned long)&dsk270_read_sector;
        *writep = (unsigned int)(unsigned long)&dsk270_write_sector;
        return 0;
}

/** @brief Select root block services and install the blockset abstraction. */
void
blockset_minit(void)
{
        kword_t name;
        kword_t super_a;
        kword_t super_b;
        unsigned int base;
        unsigned int members;
        unsigned int service;
        unsigned int read_addr;
        unsigned int write_addr;

        name = (kword_t)SIXBIT("BSET  ");
        members = blockset_boot_member_count_hint();
        if (members == 0U)
                return;
        if (root_block_services(&read_addr, &write_addr) != 0) {
                minit_diag_nodrv(name);
                return;
        }
        if (members > 1U) {
                base = minit_install(name);
                service = minit_export(name, base, 0U);
                blockset_state_addr = minit_export(name, base, 1U);
                blockset_read_addr = minit_export(name, base, 2U);
                blockset_write_addr = minit_export(name, base, 3U);
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 4U), read_addr);
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 5U), write_addr);
                storage_patch_jump(&blockset_runtime_service_jump, service);
                module_service_set(MODULE_SERVICE_BLOCKSET, service);
                minit_diag_loaded(name);
        }
        /* Discover/configure now, while BADMAP's later MINIT can still inspect
         * the validated physical layout before D6FS is installed. */
        (void)blockset_boot_discover(&super_a, &super_b);
}

#if KINIT_FULL
/** Discover raw auxiliary backing after all block drivers are resident. */
void
auxstore_minit(void)
{
        (void)auxstore_boot_discover();
        if (auxstore_logstore_blocks != 0UL) {
                logstore_boot_configure(auxstore_logstore_start,
                    auxstore_logstore_blocks);
        }
}

/** @brief Install runtime logstore over the selected writable block backend. */
void
logstore_minit(void)
{
        struct logstore recovered;
        static kword_t scratch[BLOCKSET_BLOCK_WORDS];
        kword_t *state;
        kword_t base_block;
        kword_t blocks;
        kword_t tail;
        kword_t packed;
        kword_t name;
        unsigned int base;
        unsigned int members;
        unsigned int read_addr;
        unsigned int write_addr;
        unsigned int service;
        unsigned int unit;

        if (logstore_boot_blocks() < 3UL)
                return;
        if (logstore_recover(&recovered, scratch) != 0) {
                minit_diag_notok((kword_t)SIXBIT("LOGSTR"));
                return;
        }
        members = blockset_boot_member_count_hint();
        if (auxstore_logstore_blocks == 0UL && members == 0U)
                return;
        name = (kword_t)SIXBIT("LOGSTR");
        base = minit_install(name);
        service = minit_export(name, base, LOGSTORE_X_DISPATCH);
        state = (kword_t *)(unsigned long)minit_export(name, base,
            LOGSTORE_X_STATE);
        if (auxstore_logstore_blocks != 0UL) {
                if (auxstore_kind == AUXSTORE_KIND_DRM) {
                        read_addr = module_service_get(MODULE_SERVICE_DRM_READ_BLOCK);
                        write_addr = module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK);
                } else {
                        read_addr = module_service_get(MODULE_SERVICE_DSK_READ_SECTOR);
                        write_addr = module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR);
                }
                if (read_addr == 0U || write_addr == 0U)
                        minit_fatal(name);
                state[0] = 0UL;
                packed = ((kword_t)(auxstore_unit + 1U) << 18U) |
                    auxstore_logstore_start;
        } else if (members == 1U) {
                if (!blockset_boot_member(0U, &unit, &base_block, &blocks,
                    &tail) || root_block_services(&read_addr, &write_addr) != 0)
                        minit_fatal(name);
                packed = ((kword_t)(unit + 1U) << 18) | base_block;
                state[0] = logstore_boot_start_block();
        } else {
                if (blockset_read_addr == 0U || blockset_write_addr == 0U)
                        minit_fatal(name);
                read_addr = blockset_read_addr;
                write_addr = blockset_write_addr;
                packed = 0UL;
                state[0] = logstore_boot_start_block();
        }
        state[1] = logstore_boot_blocks();
        state[2] = recovered.next_sequence;
        state[3] = ((kword_t)recovered.capacity << 18) |
            (kword_t)recovered.next_slot;
        state[4] = packed;
        state[5] = 0UL;
        state[6] = 0UL;
        storage_patch_module_jump(base,
            (kword_t *)(unsigned long)minit_export(name, base,
            LOGSTORE_X_READ_JUMP), read_addr);
        storage_patch_module_jump(base,
            (kword_t *)(unsigned long)minit_export(name, base,
            LOGSTORE_X_WRITE_JUMP), write_addr);
        module_service_set(MODULE_SERVICE_LOGSTORE, service);
        storage_patch_jump(&sys_logstore_service_jump, service);
        minit_diag_loaded(name);
}
#endif

#if KINIT_BADMAP

/** @brief Install bad-block translation state over the selected block backend. */
void
badmap_minit(void)
{
        kword_t name;
        kword_t base_block;
        kword_t blocks;
        kword_t tail;
        kword_t *entries;
        kword_t *state;
        unsigned int base;
        unsigned int count;
        unsigned int i;
        unsigned int members;
        unsigned int unit;
        unsigned int read_addr;
        unsigned int write_addr;
        unsigned int service;

        count = blockset_boot_badmap_count();
        if (count == 0U)
                return;
        entries = blockset_boot_badmap_staged();
        if (blockset_boot_badmap_load(entries, count) != 0) {
                minit_diag_notok((kword_t)SIXBIT("BADMAP"));
                return;
        }
        name = (kword_t)SIXBIT("BADMAP");
        members = blockset_boot_member_count_hint();
        base = minit_install(name);
        service = minit_export(name, base, 0U);
        state = (kword_t *)(unsigned long)minit_export(name, base, 1U);
        badmap_runtime_state = state;
        blockset_read_addr = minit_export(name, base, 2U);
        blockset_write_addr = minit_export(name, base, 3U);
        if (root_block_services(&read_addr, &write_addr) != 0) {
                minit_diag_nodrv(name);
                return;
        }
        storage_patch_module_jump(base, (kword_t *)(unsigned long)
            minit_export(name, base, 4U), read_addr);
        storage_patch_module_jump(base, (kword_t *)(unsigned long)
            minit_export(name, base, 5U), write_addr);
        state[0] = (kword_t)count;
        /* KINIT staging is valid through module_run_minits().  The post-MINIT
         * finalizer copies these words into managed runtime storage only after
         * the packed permanent MRES block is complete. */
        state[1] = (kword_t)(unsigned long)entries;
        state[2] = (kword_t)members;
        for (i = 0U; i < members; ++i) {
                if (!blockset_boot_member(i, &unit, &base_block, &blocks, &tail)) {
                        minit_diag_notok(name);
                        return;
                }
                state[3U + i] = (kword_t)unit;
                if (i == 0U && members == 1U) {
                        state[7] = base_block;
                        state[8] = blocks + tail;
                }
        }
        if (members > 1U)
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 6U),
                    module_service_get(MODULE_SERVICE_BLOCKSET));
        storage_patch_jump(&blockset_runtime_service_jump, service);
        module_service_set(MODULE_SERVICE_BLOCKSET, service);
        minit_diag_loaded(name);
}

/** @brief Publish final badmap/blockset bindings after storage MINIT completion. */
void
badmap_post_minits(void)
{
        kword_t table_base;
        kword_t *dst;
        kword_t *src;
        unsigned int count;
        unsigned int i;

        if (badmap_runtime_state == 0)
                return;
        count = (unsigned int)badmap_runtime_state[0];
        src = (kword_t *)(unsigned long)badmap_runtime_state[1];
        if (badmap_runtime_state[2] != 1UL || badmap_runtime_state[3] != 0UL ||
            badmap_runtime_state[7] == 0UL || badmap_runtime_state[8] == 0UL)
                minit_diag_notok((kword_t)SIXBIT("BMGEO "));
        if (count == 0U || src == 0 ||
            mm_alloc((kword_t)count, MM_TYPE_KERNEL_DYNAMIC, BADMAP_MM_OWNER,
            MM_ALLOC_LOW, &table_base) != MM_OK)
                kinit_halt();
        dst = (kword_t *)(unsigned long)table_base;
        {
                kword_t first_word;
                first_word = src[0];
        /* mm_alloc() does not alter payload words.  Copy overlap-safely because
         * the newly allocated low extent may cover the reclaimable KINIT
         * staging block itself. */
        if (dst > src && dst < src + count) {
                i = count;
                while (i != 0U) {
                        --i;
                        dst[i] = src[i];
                }
        } else {
                for (i = 0U; i < count; ++i)
                        dst[i] = src[i];
        }
                if (dst[0] != first_word)
                        minit_diag_notok((kword_t)SIXBIT("BMTBL "));
        }
        badmap_runtime_state[1] = table_base;
}
#endif

/** @brief Install D6FS over blockset and create the boot-root reader state. */
void
d6fs_minit(void)
{
        kword_t name;
        kword_t backing_ops;
        unsigned int base;
        unsigned int members;
        unsigned int read_addr;
        unsigned int write_addr;
        unsigned int callback_read;
        unsigned int callback_write;

        name = (kword_t)SIXBIT("D6FS  ");
        members = blockset_boot_member_count_hint();
        if (members == 0U)
                return;
        if (blockset_read_addr != 0U || blockset_write_addr != 0U) {
                /* BLOCKSET and optional BADMAP publish the logical backing
                 * callbacks here.  A singleton without either package keeps
                 * the direct DSK path and pays no resident dispatch cost. */
                read_addr = blockset_read_addr;
                write_addr = blockset_write_addr;
        } else if (members == 1U) {
                if (root_block_services(&read_addr, &write_addr) != 0) {
                        minit_diag_nodrv(name);
                        return;
                }
        } else {
                minit_diag_nodrv(name);
                return;
        }
        if (read_addr == 0U || write_addr == 0U) {
                minit_diag_nodrv(name);
                return;
        }
        base = minit_install(name);
        d6fs_backing_read_addr = minit_export(name, base, 1U);
        d6fs_backing_write_addr = minit_export(name, base, 2U);
        callback_read = minit_export(name, base, 3U);
        callback_write = minit_export(name, base, 4U);
        if (members == 1U && blockset_read_addr == 0U) {
                unsigned int direct_read;
                unsigned int direct_write;

                direct_read = minit_export(name, base, 5U);
                direct_write = minit_export(name, base, 6U);
                backing_ops = ((kword_t)direct_read << 18U) |
                    (kword_t)direct_write;
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 7U), read_addr);
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 8U), write_addr);
        } else {
                backing_ops = ((kword_t)d6fs_backing_read_addr << 18U) |
                    (kword_t)d6fs_backing_write_addr;
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)callback_read, read_addr);
                storage_patch_module_jump(base,
                    (kword_t *)(unsigned long)callback_write, write_addr);
        }
        /* The per-mount direct adapter is also used by secondary D6FS
         * instances.  Patch DRM independently of the root backing so a
         * multi-DSK root does not disable later direct or interleaved DRM
         * mounts.  Unpatched slots fail safely in the MRES. */
        read_addr = module_service_get(MODULE_SERVICE_DRM_READ_BLOCK);
        write_addr = module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK);
        if (read_addr != 0U)
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 9U),
                    (unsigned int)(unsigned long)&drm236_read_block);
        if (write_addr != 0U)
                storage_patch_module_jump(base, (kword_t *)(unsigned long)
                    minit_export(name, base, 10U),
                    (unsigned int)(unsigned long)&drm236_write_block);
        if (d6fs_reader_bootstrap_call(backing_ops) != 0) {
                minit_diag_notok(name);
                return;
        }
        {
                unsigned int service;

                service = minit_export(name, base, 0U);
                storage_patch_jump(&fs_d6fs_service_jump, service);
                module_service_set(MODULE_SERVICE_D6FS, service);
        }
        minit_diag_loaded(name);
}

/** @brief Register MonitorFS device namespace after module services are known. */
void
mfsdev_minit(void)
{
        if (module_service_get(MODULE_SERVICE_CTY_PUTCHAR) != 0U &&
            module_service_get(MODULE_SERVICE_CTY_GETCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_CTY0] = (kword_t)SIXBIT("CTY0  ");
        if (module_service_get(MODULE_SERVICE_CLK_TICKS) != 0U)
                mfsdev_names[MONITORFS_DEV_CLK0] = (kword_t)SIXBIT("CLK0  ");
        if (module_service_get(MODULE_SERVICE_PTR_GETCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_PTR0] = (kword_t)SIXBIT("PTR0  ");
        if (module_service_get(MODULE_SERVICE_PTP_PUTCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_PTP0] = (kword_t)SIXBIT("PTP0  ");
        if (module_service_get(MODULE_SERVICE_LPT_PUTCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_LPT0] = (kword_t)SIXBIT("LPT0  ");
        if (module_service_get(MODULE_SERVICE_CR_READ_CARD) != 0U)
                mfsdev_names[MONITORFS_DEV_CR0] = (kword_t)SIXBIT("CR0   ");
        if (module_service_get(MODULE_SERVICE_CP_PUNCH_CARD) != 0U)
                mfsdev_names[MONITORFS_DEV_CP0] = (kword_t)SIXBIT("CP0   ");
        if (module_service_get(MODULE_SERVICE_DCS_GETCHAR) != 0U &&
            module_service_get(MODULE_SERVICE_DCS_PUTCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_DCS0] = (kword_t)SIXBIT("DCS0  ");
        if (module_service_get(MODULE_SERVICE_GE_GETCHAR) != 0U &&
            module_service_get(MODULE_SERVICE_GE_PUTCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_GE0] = (kword_t)SIXBIT("GE0   ");
        if (module_service_get(MODULE_SERVICE_DPY_PUTWORD) != 0U)
                mfsdev_names[MONITORFS_DEV_DPY0] = (kword_t)SIXBIT("DPY0  ");
        if (module_service_get(MODULE_SERVICE_TTY_PUTCHAR) != 0U)
                mfsdev_names[MONITORFS_DEV_TTY0] = (kword_t)SIXBIT("TTY0  ");
        if (module_service_get(MODULE_SERVICE_WCNSLS_READ) != 0U)
                mfsdev_names[MONITORFS_DEV_WCNSLS] = (kword_t)SIXBIT("WCNSLS");
        if (module_service_get(MODULE_SERVICE_OCNSLS_READ) != 0U)
                mfsdev_names[MONITORFS_DEV_OCNSLS] = (kword_t)SIXBIT("OCNSLS");
        if (module_service_get(MODULE_SERVICE_DTC_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DTC_WRITE_BLOCK) != 0U)
                mfsdev_names[MONITORFS_DEV_DTC0] = (kword_t)SIXBIT("DTC0  ");
        if (module_service_get(MODULE_SERVICE_MTC) != 0U)
                mfsdev_names[MONITORFS_DEV_MTC0] = (kword_t)SIXBIT("MTC0  ");
        if (module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) != 0U &&
            module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) != 0U)
                mfsdev_names[MONITORFS_DEV_DSK0] = (kword_t)SIXBIT("DSK0  ");
        if (module_service_get(MODULE_SERVICE_SLV_HANDLER) != 0U)
                mfsdev_names[MONITORFS_DEV_SLV0] = (kword_t)SIXBIT("SLV0  ");
        if (module_service_get(MODULE_SERVICE_D6FS) != 0U)
                mfsdev_names[MONITORFS_DEV_D6SET0] = (kword_t)SIXBIT("D6SET0");
        if (module_service_get(MODULE_SERVICE_DRM_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK) != 0U)
                mfsdev_names[MONITORFS_DEV_DRM0] = (kword_t)SIXBIT("DRM0  ");
}

#if KINIT_FULL
/** @brief Probe/install SLV and publish its interrupt handler service. */
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
                module_service_set(MODULE_SERVICE_SLV_HANDLER, handler);
        }
        minit_diag_ok(name);
}
#endif
