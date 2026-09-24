#ifndef DAIMON_MODULE_H
#define DAIMON_MODULE_H

#include "kinit.h"

extern kword_t __minit_table_begin;
extern kword_t __minit_table_end;

#define MODULE_SERVICE_PTP_PUTCHAR      1U
#define MODULE_SERVICE_CR_READ_CARD     2U
#define MODULE_SERVICE_CP_PUNCH_CARD    3U
#define MODULE_SERVICE_WCNSLS_READ      4U
#define MODULE_SERVICE_CLK_TICKS        5U
#define MODULE_SERVICE_PTR_GETCHAR      6U
#define MODULE_SERVICE_OCNSLS_READ      7U
#define MODULE_SERVICE_DCS_GETCHAR       8U
#define MODULE_SERVICE_DCS_PUTCHAR       9U
#define MODULE_SERVICE_TTY_PUTCHAR      10U
#define MODULE_SERVICE_GE_GETCHAR       11U
#define MODULE_SERVICE_GE_PUTCHAR       12U
#define MODULE_SERVICE_DPY_PUTWORD       13U
#define MODULE_SERVICE_DTC_READ_BLOCK    14U
#define MODULE_SERVICE_MTC              15U
#define MODULE_SERVICE_DSK_READ_SECTOR   16U
#define MODULE_SERVICE_DSK_WRITE_SECTOR  17U
#define MODULE_SERVICE_DTC_WRITE_BLOCK   18U
#define MODULE_SERVICE_CTY_PUTCHAR       19U
#define MODULE_SERVICE_CTY_GETCHAR       20U
#define MODULE_SERVICE_MEMFS             21U
#define MODULE_SERVICE_DTFS              22U
#define MODULE_SERVICE_D6FS              23U
#define MODULE_SERVICE_SLV_HANDLER       24U
#define MODULE_SERVICE_BLOCKSET           25U
#define MODULE_SERVICE_DRM_READ_BLOCK    26U
#define MODULE_SERVICE_DRM_WRITE_BLOCK   27U
#define MODULE_SERVICE_LPT_PUTCHAR       28U
#define MODULE_SERVICE_COUNT             29U

void module_run_minits(void);
void badmap_post_minits(void);
const kword_t *module_current_mres(void);
void module_service_set(unsigned int service, unsigned int address);
unsigned int module_service_get(unsigned int service);

void module_pi_init(void);
int module_pi_register(unsigned int level, unsigned int handler);
int module_pi_unregister(unsigned int level, unsigned int handler);
void minit_pi_low_init(void);
void minit_pi_hw_clear(void);
void minit_pi_hw_set(kword_t mask);
void minit_pi_request(kword_t mask);


/* Disposable MINIT-only raw I/O probes. */
kword_t minit_cty_coni(void);
void minit_cty_cono(kword_t word);
kword_t minit_clk_coni(void);
void minit_clk_cono(kword_t word);
kword_t minit_ptr_coni(void);
void minit_ptr_cono(kword_t word);
kword_t minit_ptp_coni(void);
void minit_ptp_cono(kword_t word);
kword_t minit_lpt_coni(void);
void minit_lpt_cono(kword_t word);
kword_t minit_cr_coni(void);
void minit_cr_cono(kword_t word);
kword_t minit_cp_coni(void);
void minit_cp_cono(kword_t word);
kword_t minit_dcs_coni(void);
void minit_dcs_cono(kword_t word);
kword_t minit_gtyi_coni(void);
void minit_gtyi_cono(kword_t word);
kword_t minit_gtyo_coni(void);
void minit_gtyo_cono(kword_t word);
kword_t minit_dpy_coni(void);
void minit_dpy_cono(kword_t word);
void minit_wcnsls_cono(kword_t word);
kword_t minit_wcnsls_datai(void);
void minit_wcnsls_plot(kword_t word);
kword_t minit_storage_probe(unsigned int kind);
kword_t minit_drm236_probe(void);
kword_t minit_slv_coni(void);
void minit_slv_cono(kword_t word);

#endif
