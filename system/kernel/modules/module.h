/**
 * @file module.h
 * @brief KINIT module discovery, service publication, and MINIT I/O interface.
 *
 * Built-in modules are boot-time MINIT/MRES pairs. MINIT code probes hardware,
 * installs the associated resident MRES package when needed, registers PI
 * handlers, and publishes resident service entry points through a transient
 * KINIT service directory. Raw minit_* I/O helpers exist only before resident
 * drivers are available and are reclaimed with KINIT.
 */
#ifndef DAIMON_MODULE_H
#define DAIMON_MODULE_H

#include "kinit.h"

/** Linker-delimited table of built-in MINIT/MRES pairs. */
extern kword_t __minit_table_begin;
extern kword_t __minit_table_end;

/** Boot-time service indexes; zero means "service not published". */
#define MODULE_SERVICE_DCS_GETCHAR        1U
#define MODULE_SERVICE_DCS_PUTCHAR        2U
#define MODULE_SERVICE_GE_GETCHAR         3U
#define MODULE_SERVICE_GE_PUTCHAR         4U
#define MODULE_SERVICE_DTC_READ_BLOCK     5U
#define MODULE_SERVICE_DSK_READ_SECTOR    6U
#define MODULE_SERVICE_DSK_WRITE_SECTOR   7U
#define MODULE_SERVICE_DTC_WRITE_BLOCK    8U
#define MODULE_SERVICE_CTY_GETCHAR        9U
#define MODULE_SERVICE_BLOCKSET          10U
#define MODULE_SERVICE_DRM_READ_BLOCK    11U
#define MODULE_SERVICE_DRM_WRITE_BLOCK   12U
#define MODULE_SERVICE_DPY_PUTCHAR       13U
#define MODULE_SERVICE_COUNT             14U

/** Execute every built-in MINIT/MRES pair in table order. */
void module_run_minits(void);
void dtfs_post_minits(void);
void auxstore_post_minits(void);
void badmap_post_minits(void);
/** Return the MRES package associated with the MINIT currently executing. */
const kword_t *module_current_mres(void);
/** Publish one resident service entry point during KINIT. */
void module_service_set(unsigned int service, unsigned int address);
/** Return a published resident service address, or zero when unavailable. */
unsigned int module_service_get(unsigned int service);

/** Install low-core PI vectors and initialize transient PI registration state. */
void module_pi_init(void);
/** Add one resident handler to a PDP-6 PI level. */
int module_pi_register(unsigned int level, unsigned int handler);
/** Remove one previously registered resident PI handler. */
int module_pi_unregister(unsigned int level, unsigned int handler);
/** Replace one handler in place without opening an interrupt-unhandled gap. */
int module_pi_replace(unsigned int level, unsigned int old_handler,
    unsigned int new_handler);
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
