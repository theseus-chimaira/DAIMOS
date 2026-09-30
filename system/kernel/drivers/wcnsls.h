/**
 * @file wcnsls.h
 * @brief PDP-6 MIT Spacewar console and color-scope interface.
 *
 * WCNSLS device 0420 combines four active-low Spacewar control banks with the
 * optional DEC color-scope point plotter. KINIT enables Spacewar input and uses
 * the active-low idle value to distinguish real hardware from the null-device
 * all-zero DATAI path. When present, one tiny MRES exposes raw switch input,
 * CONO control, and coordinate DATAO plotting.
 *
 * Each player occupies one nine-bit field in the 36-bit input word. Defined
 * switch bits are active low: WCNSLS_PRESSED() therefore tests for a cleared
 * bit. Scope coordinates are unsigned 9-bit X/Y values packed into one word.
 */
#ifndef DAIMON_WCNSLS_H
#define DAIMON_WCNSLS_H

#include "kcore.h"

/** MIT Spacewar console/color-scope I/O device number. */
#define WCNSLS_DEVICE           0420U
/** CONO bit 30: enable Spacewar switch-bank DATAI. */
#define WCNSLS_CO_SPACEWAR      0000040UL
/** CONO bit 24: latch/update green scope intensity. */
#define WCNSLS_CO_GREEN_ENABLE  0002000UL
/** CONO bits 25..28: four-bit green intensity field. */
#define WCNSLS_CO_GREEN_MAX     0001700UL
/** Full-intensity green scope selection used by the boot banner. */
#define WCNSLS_CO_GREEN_FULL    (WCNSLS_CO_GREEN_ENABLE | WCNSLS_CO_GREEN_MAX)
/** Pack 9-bit X and Y color-scope coordinates for DATAO. */
#define WCNSLS_COORD(x,y)       ((((kword_t)(x) & 0777UL) << 9) | \
                                 ((kword_t)(y) & 0777UL))

/** Upper-right player field shift. */
#define WCNSLS_POS_UR           0U
/** Lower-right player field shift. */
#define WCNSLS_POS_LR           9U
/** Lower-left player field shift. */
#define WCNSLS_POS_LL           18U
/** Upper-left player field shift. */
#define WCNSLS_POS_UL           27U
/** Active-low counter-clockwise switch bit within one player field. */
#define WCNSLS_SW_CCW           0400UL
/** Active-low clockwise switch bit. */
#define WCNSLS_SW_CW            0200UL
/** Active-low thrust switch bit. */
#define WCNSLS_SW_THRUST        0100UL
/** Active-low hyperspace switch bit. */
#define WCNSLS_SW_HYPER         0040UL
/** Active-low fire switch bit. */
#define WCNSLS_SW_FIRE          0020UL
/** Form one player-specific active-low switch mask. */
#define WCNSLS_BIT(pos, sw)     (((kword_t)(sw)) << (pos))
/** Return nonzero when one player-specific active-low switch is pressed. */
#define WCNSLS_PRESSED(raw,pos,sw) ((((raw) & WCNSLS_BIT((pos),(sw))) == 0UL) ? 1 : 0)

/** @brief Read the raw four-player active-low Spacewar switch word. */
kword_t wcnsls_read(void);
/** @brief Write the raw WCNSLS CONO control/color word. */
void wcnsls_cono(kword_t word);
/** @brief Plot one packed WCNSLS_COORD() point on the optional color scope. */
void wcnsls_plot(kword_t word);

#endif
