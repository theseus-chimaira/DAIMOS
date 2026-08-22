#ifndef DAIMON_MODULE_H
#define DAIMON_MODULE_H

#include "kinit.h"

extern kword_t __minit_table_begin;
extern kword_t __minit_table_end;

#define MODULE_SERVICE_PTP_PUTCHAR      1U
#define MODULE_SERVICE_CR_READ_CARD     2U
#define MODULE_SERVICE_CP_PUNCH_CARD    3U
#define MODULE_SERVICE_WCNSLS_READ      4U
#define MODULE_SERVICE_COUNT            5U

void module_run_minits(void);
const kword_t *module_current_mres(void);
void module_service_set(unsigned int service, unsigned int address);
unsigned int module_service_get(unsigned int service);


/* Disposable MINIT-only raw I/O probes. */
kword_t minit_cty_coni(void);
void minit_cty_cono(kword_t word);
kword_t minit_clk_coni(void);
void minit_clk_cono(kword_t word);
kword_t minit_ptr_coni(void);
void minit_ptr_cono(kword_t word);
kword_t minit_ptp_coni(void);
void minit_ptp_cono(kword_t word);
kword_t minit_cr_coni(void);
void minit_cr_cono(kword_t word);
kword_t minit_cp_coni(void);
void minit_cp_cono(kword_t word);
void minit_wcnsls_cono(kword_t word);
kword_t minit_slv_coni(void);
void minit_slv_cono(kword_t word);

#endif
