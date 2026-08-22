#ifndef DAIMON_KINIT_H
#define DAIMON_KINIT_H

#include <pdp10-sixbit.h>

/* PDP-6/PDP-10 C uses one 36-bit word for unsigned long. */
#ifndef DAIMON_KWORD_T_DEFINED
#define DAIMON_KWORD_T_DEFINED
typedef unsigned long kword_t;
#endif

#define KINIT_WORD_MASK          0777777777777UL
#define KINIT_HALF_MASK          0777777UL
#define KINIT_KCORE_BASE         000060UL
#define KINIT_BOOT_SIXBIT_BASE   077760UL
#define KINIT_BOOT_WORD0         000040UL
#define KINIT_BOOT_WORD1         000041UL
#define KINIT_MACHINE_NAME       PDP10_SIXBIT4('P','D','P','6')

#define KINIT_LH(w) \
        ((unsigned int)(((w) >> 18) & KINIT_HALF_MASK))
#define KINIT_RH(w) \
        ((unsigned int)((w) & KINIT_HALF_MASK))

extern kword_t __kinit_image_start;
extern kword_t __kinit_image_end;

void kinit_save_boot_handoff(void);

void kinit_diag_banner(void);
void kinit_diag_system(void);
void kinit_diag_finished(void);

void kinit_put6(kword_t word);
void kinit_newline(void);
void kinit_call18(unsigned int address);
void kinit_halt(void);

#ifdef KINIT_DEBUG
#define KINIT_TRACE(name) do { kinit_put6((kword_t)SIXBIT(name)); kinit_newline(); } while (0)
#else
#define KINIT_TRACE(name) ((void)0)
#endif

#endif
