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
/** CONO bit 31: latch/update blue scope intensity. */
#define WCNSLS_CO_BLUE_ENABLE   0000020UL
/** CONO bits 32..35: four-bit blue intensity field. */
#define WCNSLS_CO_BLUE_MAX      0000017UL
/** CONO bit 24: latch/update green scope intensity. */
#define WCNSLS_CO_GREEN_ENABLE  0002000UL
/** CONO bits 25..28: four-bit green intensity field. */
#define WCNSLS_CO_GREEN_MAX     0001700UL
/** CONO bit 18: latch/update red scope intensity. */
#define WCNSLS_CO_RED_ENABLE    0200000UL
/** CONO bits 19..22: four-bit red intensity field. */
#define WCNSLS_CO_RED_MAX       0170000UL
/** Full-intensity green scope selection used by the boot banner. */
#define WCNSLS_CO_GREEN_FULL    (WCNSLS_CO_GREEN_ENABLE | WCNSLS_CO_GREEN_MAX)
/** Pack 9-bit X and Y color-scope coordinates for DATAO. */
#define WCNSLS_COORD(x,y)       ((((kword_t)(x) & 0777UL) << 9) | \
                                 ((kword_t)(y) & 0777UL))

/*
 * /DEV/WCNSLS raw word stream.  Privileged userspace owns refresh and device
 * policy; the kernel merely executes batched device-0420 operations.  A clear
 * tag is DATAO and a set tag is CONO.  Only the hardware's 18-bit I/O word is
 * carried in the payload.
 */
#define WCNSLS_RAW_CONO         0400000000000UL
#define WCNSLS_RAW_DATA_MASK    0000000777777UL
#define WCNSLS_RAW_DATAO(word)  ((kword_t)(word) & 0377777777777UL)
#define WCNSLS_RAW_CONO_WORD(word) \
        (WCNSLS_RAW_CONO | ((kword_t)(word) & WCNSLS_RAW_DATA_MASK))

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

/** @brief Read up to nwords raw DATAI samples into a privileged user buffer. */
int wcnsls_read_words(kword_t *buf, unsigned int nwords);
/** @brief Execute a batch of tagged raw CONO/DATAO operations. */
int wcnsls_write_words(const kword_t *buf, unsigned int nwords);

#endif
