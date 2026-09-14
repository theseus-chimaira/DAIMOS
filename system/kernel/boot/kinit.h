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
void kinit_call18(unsigned int address);
kword_t kinit_call18_0(unsigned int address);
kword_t kinit_call18_1(unsigned int address, kword_t arg);
kword_t kinit_call_fs_request(unsigned int address, const void *req);
kword_t kinit_call_diskset_request(unsigned int address, const void *req);
kword_t kinit_call18_2(unsigned int address, kword_t arg1, kword_t arg2);
kword_t kinit_call18_3(unsigned int address, kword_t arg1, kword_t arg2,
    kword_t arg3);
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
