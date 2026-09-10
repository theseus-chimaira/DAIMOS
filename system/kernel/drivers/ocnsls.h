#ifndef DAIMON_OCNSLS_H
#define DAIMON_OCNSLS_H

#include "kcore.h"

#define OCNSLS_DEVICE           0724U
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

kword_t ocnsls_read(void);

#endif
