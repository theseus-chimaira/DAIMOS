#ifndef DAIMON_DTC_H
#define DAIMON_DTC_H

#define DTC_NATIVE_PI_LEVEL     5U
#define DTC_ST_PI_MASK          0000007UL
#define DTC_CO_UNIT_SHIFT       3U
#define DTC_CO_READ             0000300UL
#define DTC_CO_START            0020000UL
#define DTC_CO_JOB_DONE         0040000UL
#define DTC_CO_SELECT           0200000UL
#define DTC_CO_STOP             0000000UL
#define DTC_DCT_INPUT           0004000UL
#define DTC_DCT_DEVICE          0000040UL
#define DTC_DCT_REQUEST         0001000UL
#define DTC_STB_ERROR_MASK      0000034UL
#define DTC_E_BUSY              (-3)
#define DTC_E_ARG               (-1)
#define DTC_E_IO                (-5)

#endif
