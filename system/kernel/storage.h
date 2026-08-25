#ifndef DAIMON_STORAGE_H
#define DAIMON_STORAGE_H

#define STORAGE_NATIVE_PI_LEVEL    5U
#define STORAGE_ST_PI_MASK         0000007UL
#define STORAGE_DCT_REQUEST        0001000UL
#define STORAGE_DCT_ACCEPT         0002000UL
#define STORAGE_DCT_INPUT          0004000UL
#define STORAGE_DCT_DTC_DEVICE     0000040UL
#define STORAGE_DCT_MTC_DEVICE     0000000UL

#define DTC_STB_ERROR_MASK      0000034UL
#define DTC_CO_UNIT_SHIFT       3U
#define DTC_CO_READ             0000300UL
#define DTC_CO_START            0020000UL
#define DTC_CO_JOB_DONE         0040000UL
#define DTC_CO_SELECT           0200000UL

#define MTC_CO_READ_556_BINARY  0052400UL
#define MTC_ST_EOR              0000004UL
#define MTC_ST_ERROR_MASK       00400520UL
#define MTC_SO_ENABLE_EOR       0000004UL
#define MTC_CO_UNIT_SHIFT       4U

#define DSK_WORDS_PER_SECTOR    0200U
#define DSK_ST_ERROR_MASK       0001777UL
#define DSK_ST_DFR              0040000UL
#define DSK_ST_IDS              0400000UL
#define DSK_CO_READ             0001000UL
#define DSK_CO_WRITE            0002000UL
#define DSK_CO_ENABLE_ERROR     0000100UL
#define DSK_CO_END_CLEAR        0030000UL

#define STORAGE_E_ARG              (-1)
#define STORAGE_E_BUSY             (-3)
#define STORAGE_E_IO               (-5)

#endif
