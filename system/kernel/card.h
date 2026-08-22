#ifndef DAIMON_CARD_H
#define DAIMON_CARD_H

#include "kcore_pi.h"

#define CP_DEVICE               0110U
#define CR_DEVICE               0150U
#define CARD_NATIVE_PI_LEVEL    7U
#define CARD_COLUMNS            80U
#define CARD_COLUMN_MASK        07777UL
#define CARD_WAIT_READY         0200000U

#define CR_CO_CLR_DRDY          0000010UL
#define CR_CO_CLR_END_CARD      0000020UL
#define CR_CO_CLR_DATA_MISS     0000200UL
#define CR_CO_READ_CARD         0001000UL
#define CR_CO_CLR_READER        0010000UL
#define CR_ST_PI_MASK           0000007UL
#define CR_ST_DATA_RDY          0000010UL
#define CR_ST_END_CARD          0000020UL
#define CR_ST_RDY_READ          0000100UL
#define CR_ST_TROUBLE           0000400UL

#define CP_CO_SET_PUNCH_ON      0000040UL
#define CP_CO_CLR_END_CARD      0000100UL
#define CP_CO_EN_END_CARD       0000200UL
#define CP_CO_CLR_ERROR         0001000UL
#define CP_CO_EJECT             0010000UL
#define CP_CO_CLR_PUNCH         0100000UL
#define CP_ST_PI_MASK           0000007UL
#define CP_ST_DATA_REQ          0000010UL
#define CP_ST_END_CARD          0000100UL
#define CP_ST_ERROR             0001000UL
#define CP_ST_TROUBLE           0004000UL

#define CARD_E_ARG             -1
#define CARD_E_TIMEOUT         -2
#define CARD_E_IO              -3
#define CARD_E_BUSY            -4
#define CARD_E_LIMIT           -5

int cr_read_card(kword_t cols[CARD_COLUMNS]);
int cp_punch_card(const kword_t cols[CARD_COLUMNS]);
int cr_pi_handler(unsigned int level, kword_t opaque);
int cp_pi_handler(unsigned int level, kword_t opaque);

kword_t cr_coni(void);
void cr_cono(kword_t word);
kword_t cr_datai(void);
kword_t cp_coni(void);
void cp_cono(kword_t word);
void cp_datao(kword_t word);

#endif
