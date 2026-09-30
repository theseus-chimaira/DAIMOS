#ifndef DAIMON_KINIT_H
#define DAIMON_KINIT_H

#include <pdp10-sixbit.h>

/* PDP-6/PDP-10 C uses one 36-bit word for unsigned long. */
#ifndef DAIMON_KWORD_T_DEFINED
#define DAIMON_KWORD_T_DEFINED
typedef unsigned long kword_t;
#endif

/*
 * Core-layout constants used while KINIT constructs the permanent kernel.
 *
 * PDP-10 addresses are 18-bit word addresses.  KCORE begins at 060, leaving
 * the low monitor/trap area below it available for fixed boot interfaces.
 * KINIT itself is linked at 030000 and is reclaimed after initialization.
 * Its bootstrap pushdown list occupies the fixed reserve immediately above
 * the linked KINIT image until the late handoff releases both regions.
 */
#define KINIT_HALF_MASK          0777777UL
#define KINIT_KCORE_BASE         000060UL
#define KINIT_IMAGE_BASE         030000UL
#ifndef KINIT_STACK_RESERVE_WORDS
#define KINIT_STACK_RESERVE_WORDS 02000UL
#endif
/* Permanent idle-process kernel stack: 0100 octal = 64 words. */
#define KERNEL_IDLE_STACK_WORDS   00100UL
/*
 * Stage1 writes its two-word boot/root handoff at physical words 040 and 041.
 * KINIT copies these words before low memory is reused.
 */
#define KINIT_BOOT_WORD0         000040UL
#define KINIT_BOOT_WORD1         000041UL
/* Six-character machine name printed by early boot diagnostics. */
#define KINIT_MACHINE_NAME       "PDP6  "

/*
 * Console SW low five bits select the boot root:
 *
 *   bits 0..2  controller class
 *   bits 3..4  zero-based ordinal within that controller class
 *
 * KINIT reads the APR console switch register once in root_select_minit(),
 * during MINIT processing and before kinit_boot() mounts the root filesystem.
 * The operator should therefore set the switches before starting/continuing
 * KINIT (setting them before Stage1 is the safest procedure) and leave them
 * set until root selection has occurred.  Changes after that sample do not
 * affect the current boot.
 *
 * AUTO probes supported root classes in policy order: DSK, then DTC, then
 * DRM.
 *
 * Examples (low five switch bits, octal):
 *
 *   00  AUTO, ordinal 0       01  DSK0       02  DTC0       03  DRM0
 *   11  DSK1                  12  DTC1       13  DRM1
 *   21  DSK2                  22  DTC2       23  DRM2
 *
 * The ordinal is encoded by adding 010 for each increment.
 */
#define KINIT_ROOT_CLASS_MASK     0000007UL
#define KINIT_ROOT_ORD_SHIFT      3U
#define KINIT_ROOT_ORD_MASK       0000030UL
#define KINIT_ROOT_SELECT_MASK    0000037UL
#define KINIT_ROOT_AUTO           0U
#define KINIT_ROOT_DSK            1U
#define KINIT_ROOT_DTC            2U
#define KINIT_ROOT_DRM            3U

/*
 * Fatal early-boot diagnostics are one 18-bit SIXBIT halfword, printed
 * without CR/LF so they remain usable before the normal console path exists.
 *
 * 0376264 = SIXBIT "?RT" : selected root failed validation/mounting, or the
 *                          mounted root does not contain /SYSTEM/INIT.
 */
#define KINIT_ERR_RT              0376264UL  /* "?RT" */

/* Saved private copy of the two fixed Stage1 handoff words. */
extern kword_t kinit_boot_handoff[2];

/* Extract left/right 18-bit halves from one 36-bit PDP-10 word. */
#define KINIT_LH(w) \
        ((unsigned int)(((w) >> 18) & KINIT_HALF_MASK))
#define KINIT_RH(w) \
        ((unsigned int)((w) & KINIT_HALF_MASK))

/* Detect installed core size in 1024-word units. */
unsigned int kinit_memory_kwords(void);
/* Clear/normalize APR state before normal interrupt setup. */
void kinit_apr_clear(void);
/* Print the fixed early boot banner and detected-system summary. */
void kinit_diag_banner(void);
void kinit_diag_system(unsigned int memory_kwords);
#if KINIT_STACK_WATERMARK
/* Initialize and measure the disposable KINIT stack watermark area. */
void kinit_stack_watermark_begin(void);
unsigned int kinit_stack_watermark_measure(void);
#endif

/* Early console output; kinit_put6 prints one packed SIXBIT word. */
void kinit_put6(kword_t word);
void kinit_put6_spaces(unsigned int words);
void kinit_newline(void);
/* Print one 18-bit SIXBIT fatal code and halt boot. */
void kinit_error18(kword_t code);
/* Read the 36-bit PDP-6 APR console switch register. */
kword_t kinit_read_switches(void);
/*
 * Call helpers bridge C to boot-time entry points identified by 18-bit word
 * addresses.  Typed variants preserve the compact calling conventions used
 * by filesystem, blockset, and storage MINIT services.
 */
void kinit_call18(unsigned int address);
kword_t kinit_call18_1(unsigned int address, kword_t arg);
kword_t kinit_call_fs_request(unsigned int address, const void *req);
kword_t kinit_call_blockset_request(unsigned int address, const void *req);
kword_t kinit_call_blockset_io(unsigned int address, kword_t logical,
    void *buffer);
kword_t kinit_call_storage_io(unsigned int address, unsigned int unit,
    kword_t block, void *buffer);
/* Non-returning early-boot stop. */
void kinit_halt(void);
/* Install the user monitor-UUO trap path required by later boot code. */
void kinit_user_trap_init(void);
/*
 * Transfer execution from transient KINIT to permanent KCORE using the
 * permanent kernel stack, then make the supplied KINIT range reclaimable.
 */
void kcore_boot_handoff(kword_t stack_base, kword_t reclaim_base,
    kword_t reclaim_words);
/* Select/mount the boot root and verify that /SYSTEM/INIT is available. */
void kinit_boot(void);

#ifdef KINIT_DEBUG
/* Emit the final debug marker before normal boot handoff. */
void kinit_diag_finished(void);

/*
 * Debug traces contain only prepacked uppercase SIXBIT words so tracing works
 * without a formatter, dynamic storage, or normal userspace console support.
 */
#define KINIT_TRACE_KINIT_ENTER() do { \
        kinit_put6((kword_t)SIXBIT("KINIT_")); \
        kinit_put6((kword_t)SIXBIT("ENTER ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_KCORE_LOAD() do { \
        kinit_put6((kword_t)SIXBIT("KCORE_")); \
        kinit_put6((kword_t)SIXBIT("LOAD  ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_MRES_INIT() do { \
        kinit_put6((kword_t)SIXBIT("MRES_I")); \
        kinit_put6((kword_t)SIXBIT("NIT   ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_MRES_INSTALL() do { \
        kinit_put6((kword_t)SIXBIT("MRES_I")); \
        kinit_put6((kword_t)SIXBIT("NSTALL")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_KINIT_SAVE_BOOT_HANDOFF() do { \
        kinit_put6((kword_t)SIXBIT("KINIT_")); \
        kinit_put6((kword_t)SIXBIT("SAVE_B")); \
        kinit_put6((kword_t)SIXBIT("OOT_HA")); \
        kinit_put6((kword_t)SIXBIT("NDOFF ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_KINIT_DIAG_BANNER() do { \
        kinit_put6((kword_t)SIXBIT("KINIT_")); \
        kinit_put6((kword_t)SIXBIT("DIAG_B")); \
        kinit_put6((kword_t)SIXBIT("ANNER ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_KINIT_DIAG_SYSTEM() do { \
        kinit_put6((kword_t)SIXBIT("KINIT_")); \
        kinit_put6((kword_t)SIXBIT("DIAG_S")); \
        kinit_put6((kword_t)SIXBIT("YSTEM ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_MODULE_RUN_MINITS() do { \
        kinit_put6((kword_t)SIXBIT("MODULE")); \
        kinit_put6((kword_t)SIXBIT("_RUN_M")); \
        kinit_put6((kword_t)SIXBIT("INITS ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE(name) KINIT_TRACE_##name()
#endif

#endif
