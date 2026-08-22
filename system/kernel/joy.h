#ifndef DAIMON_JOY_H
#define DAIMON_JOY_H

#include "kcore.h"

#define WCNSLS_DEVICE           0420U
#define OCNSLS_DEVICE           0724U
#define WCNSLS_CO_SPACEWAR      0000040UL
#define WCNSLS_WORD_MASK        0777777777777UL
#define OCNSLS_WORD_MASK        0777777777777UL

#define WCNSLS_POS_UR           0U
#define WCNSLS_POS_LR           9U
#define WCNSLS_POS_LL           18U
#define WCNSLS_POS_UL           27U
#define WCNSLS_SW_CCW           0400UL
#define WCNSLS_SW_CW            0200UL
#define WCNSLS_SW_THRUST        0100UL
#define WCNSLS_SW_HYPER         0040UL
#define WCNSLS_SW_FIRE          0020UL
#define WCNSLS_BIT(pos, sw)     (((kword_t)(sw)) << (pos))
#define WCNSLS_PRESSED(raw,pos,sw) ((((raw) & WCNSLS_BIT((pos),(sw))) == 0UL) ? 1 : 0)

#define OCNSLS_POS_0            0U
#define OCNSLS_POS_1            18U
#define OCNSLS_SW_HYPER         0004UL
#define OCNSLS_SW_FIRE          0010UL
#define OCNSLS_SW_CW            0020UL
#define OCNSLS_SW_CCW           0040UL
#define OCNSLS_SW_SLOW          0100UL
#define OCNSLS_SW_FAST          0200UL
#define OCNSLS_SW_BEACON        020000UL
#define OCNSLS_BIT(pos, sw)     (((kword_t)(sw)) << (pos))
#define OCNSLS_PRESSED(raw,pos,sw) ((((raw) & OCNSLS_BIT((pos),(sw))) != 0UL) ? 1 : 0)

kword_t wcnsls_read(void);
kword_t ocnsls_read(void);
void wcnsls_cono(kword_t word);

#endif
