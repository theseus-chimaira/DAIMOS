/**
 * @file ocnsls.h
 * @brief PDP-6 old MIT Spacewar console-switch input interface.
 *
 * OCNSLS device 0724 returns both players' Spacewar control switches in one
 * 36-bit DATAI word. Player 0 occupies the low 18-bit half; player 1 uses the
 * identical switch layout shifted left by 18 bits.
 *
 * The device has no useful presence probe: CONI returns zero and DATAI also
 * legitimately returns zero when no switch is pressed. KINIT therefore installs
 * the three-word read MRES unconditionally; absent/null hardware behaves as an
 * all-zero switch bank without requiring resident probe state.
 */
#ifndef DAIMON_OCNSLS_H
#define DAIMON_OCNSLS_H

#include "kcore.h"

/** Old MIT Spacewar console-switch I/O device number. */
#define OCNSLS_DEVICE           0724U
/** Player 0 switch-bank shift in the 36-bit DATAI word. */
#define OCNSLS_POS_0            0U
/** Player 1 switch-bank shift in the 36-bit DATAI word. */
#define OCNSLS_POS_1            18U
/** Hyperspace button bit within one 18-bit player bank. */
#define OCNSLS_SW_HYPER         0004UL
/** Torpedo/fire button bit within one player bank. */
#define OCNSLS_SW_FIRE          0010UL
/** Clockwise rotation switch bit. */
#define OCNSLS_SW_CW            0020UL
/** Counter-clockwise rotation switch bit. */
#define OCNSLS_SW_CCW           0040UL
/** Weak-thrust switch bit. */
#define OCNSLS_SW_SLOW          0100UL
/** Strong-thrust switch bit. */
#define OCNSLS_SW_FAST          0200UL
/** Aiming-beacon switch bit. */
#define OCNSLS_SW_BEACON        020000UL
/** Form one player-specific switch bit in the raw DATAI word. */
#define OCNSLS_BIT(pos, sw)     (((kword_t)(sw)) << (pos))
/** Return nonzero when one player-specific switch is pressed. */
#define OCNSLS_PRESSED(raw,pos,sw) ((((raw) & OCNSLS_BIT((pos),(sw))) != 0UL) ? 1 : 0)

/**
 * @brief Read both old Spacewar console switch banks.
 * @return Raw 36-bit DATAI word, player 0 in LH-shift 0 and player 1 at +18.
 */
kword_t ocnsls_read(void);

#endif
