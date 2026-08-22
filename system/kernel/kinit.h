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
#define KINIT_BOOT_WORD0         000040UL
#define KINIT_BOOT_WORD1         000041UL
#define KINIT_MACHINE_NAME       "PDP6  "

#define KINIT_LH(w) \
        ((unsigned int)(((w) >> 18) & KINIT_HALF_MASK))
#define KINIT_RH(w) \
        ((unsigned int)((w) & KINIT_HALF_MASK))

void kinit_diag_banner(void);
void kinit_diag_system(void);

void kinit_put6(kword_t word);
void kinit_newline(void);
void kinit_call18(unsigned int address);
void kinit_halt(void);

#ifdef KINIT_DEBUG
void kinit_diag_finished(void);

/* Debug traces contain only prepacked uppercase SIXBIT words. */
#define KINIT_TRACE_KINIT_ENTER() do { \
        kinit_put6((kword_t)SIXBIT("KINIT_")); \
        kinit_put6((kword_t)SIXBIT("ENTER ")); \
        kinit_newline(); \
} while (0)
#define KINIT_TRACE_MRES_LOAD() do { \
        kinit_put6((kword_t)SIXBIT("MRES_L")); \
        kinit_put6((kword_t)SIXBIT("OAD   ")); \
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
