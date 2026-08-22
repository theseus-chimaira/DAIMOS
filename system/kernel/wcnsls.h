#ifndef DAIMON_WCNSLS_H
#define DAIMON_WCNSLS_H

#include "kcore.h"

#define WCNSLS_DEVICE           0420U
#define WCNSLS_CO_SPACEWAR      0000040UL

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

kword_t wcnsls_read(void);
void wcnsls_cono(kword_t word);
void wcnsls_plot(kword_t word);

#endif
