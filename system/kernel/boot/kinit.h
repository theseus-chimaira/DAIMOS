#ifndef DAIMON_KINIT_H
#define DAIMON_KINIT_H

#include <pdp10-sixbit.h>

/* PDP-6/PDP-10 C uses one 36-bit word for unsigned long. */
#ifndef DAIMON_KWORD_T_DEFINED
#define DAIMON_KWORD_T_DEFINED
typedef unsigned long kword_t;
#endif

#define KINIT_HALF_MASK          0777777UL
#define KINIT_KCORE_BASE         000060UL
#define KINIT_IMAGE_BASE         030000UL
#ifndef KINIT_STACK_RESERVE_WORDS
#define KINIT_STACK_RESERVE_WORDS 02000UL
#endif
#define KERNEL_IDLE_STACK_WORDS   00100UL
#define KINIT_BOOT_WORD0         000040UL
#define KINIT_BOOT_WORD1         000041UL
#define KINIT_MACHINE_NAME       "PDP6  "

/* Console SW low five bits are the V0.9 root selector. */
#define KINIT_ROOT_CLASS_MASK     0000007UL
#define KINIT_ROOT_ORD_SHIFT      3U
#define KINIT_ROOT_ORD_MASK       0000030UL
#define KINIT_ROOT_SELECT_MASK    0000037UL
#define KINIT_ROOT_AUTO           0U
#define KINIT_ROOT_DSK            1U
#define KINIT_ROOT_DTC            2U
#define KINIT_ROOT_DRM            3U
#define KINIT_ROOT_RAM            4U

/* Fatal early-boot diagnostics are one SIXBIT halfword, printed without CR/LF. */
#define KINIT_ERR_B1              0374221UL
#define KINIT_ERR_RT              0376264UL
#define KINIT_ERR_MT              0375564UL

extern kword_t kinit_boot_handoff[2];

#define KINIT_LH(w) \
        ((unsigned int)(((w) >> 18) & KINIT_HALF_MASK))
#define KINIT_RH(w) \
        ((unsigned int)((w) & KINIT_HALF_MASK))

unsigned int kinit_memory_kwords(void);
void kinit_apr_clear(void);
void kinit_diag_banner(void);
void kinit_diag_system(unsigned int memory_kwords);
#if KINIT_STACK_WATERMARK
void kinit_stack_watermark_begin(void);
unsigned int kinit_stack_watermark_measure(void);
#endif

void kinit_put6(kword_t word);
void kinit_put6_spaces(unsigned int words);
void kinit_newline(void);
void kinit_error18(kword_t code);
kword_t kinit_read_switches(void);
void kinit_call18(unsigned int address);
kword_t kinit_call18_1(unsigned int address, kword_t arg);
kword_t kinit_call_fs_request(unsigned int address, const void *req);
kword_t kinit_call_blockset_request(unsigned int address, const void *req);
kword_t kinit_call_blockset_io(unsigned int address, kword_t logical,
    void *buffer);
kword_t kinit_call_storage_io(unsigned int address, unsigned int unit,
    kword_t block, void *buffer);
void kinit_halt(void);
void kinit_user_trap_init(void);
void kcore_boot_handoff(kword_t stack_base, kword_t reclaim_base,
    kword_t reclaim_words);
void kinit_boot(kword_t future_free_words);

#ifdef KINIT_DEBUG
void kinit_diag_finished(void);

/* Debug traces contain only prepacked uppercase SIXBIT words. */
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
