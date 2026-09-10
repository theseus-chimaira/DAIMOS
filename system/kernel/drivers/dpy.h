#ifndef DAIMON_DPY_H
#define DAIMON_DPY_H

#include "kcore.h"

#define DPY_DEVICE              0130U
#define DPY_NATIVE_PI_LEVEL     6U
#define DPY_PROBE_SPEC_PI       5U

#define DPY_ST_SPEC_MASK        0007400UL
#define DPY_ST_DONE             0000200UL
#define DPY_DATA_PI_MASK        0000007UL
#define DPY_SPEC_PI_MASK        0000070UL
#define DPY_SPEC_PI_SHIFT       3U
#define DPY_CO_INIT             0000100UL

#define DPY_MODE_PARAM          0U
#define DPY_MODE_POINT          1U
#define DPY_MODE_CHAR           3U
#define DPY_T342_SPACE          040U

#define DPY_E_OK                0
#define DPY_E_BUSY             -3

int dpy_putword(kword_t word);
void dpy_pi_handler(void);

#endif
