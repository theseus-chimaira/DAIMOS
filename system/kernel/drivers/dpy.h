/**
 * @file dpy.h
 * @brief PDP-6 Type 340 display interface on the Type 344 controller.
 *
 * The display is optional. KINIT probes device 0130, initializes it, installs
 * the DPY MRES package, and publishes dpy_putword() only when the controller
 * responds correctly. The resident driver keeps one pending flag because the
 * Type 340 accepts one 36-bit DATAO word and raises DONE after completing its
 * second 18-bit display instruction.
 *
 * DPY shares PI6 with the APR line clock. KINIT therefore replaces the clock's
 * PI-table entry with dpy_pi_handler() and patches the embedded clock-service
 * call to the relocated CLK MRES entry when CLK is present. Special-condition
 * interrupts use a separate PIA only during probing and remain disabled during
 * normal DAIMOS operation.
 */
#ifndef DAIMON_DPY_H
#define DAIMON_DPY_H

#include "kcore.h"

/** Type 340/344 display I/O device number. */
#define DPY_DEVICE              0130U
/** Data/DONE PI level shared with the APR line clock. */
#define DPY_NATIVE_PI_LEVEL     6U
/** Temporary special-condition PIA used only to verify the probe register. */
#define DPY_PROBE_SPEC_PI       5U

/** CONI bit 28: second half of the most recent DATAO word is complete. */
#define DPY_ST_DONE             0000200UL
/** CONI/CONO bits 33..35: data/DONE PI assignment. */
#define DPY_DATA_PI_MASK        0000007UL
/** CONI/CONO bits 30..32: special-condition PI assignment. */
#define DPY_SPEC_PI_MASK        0000070UL
/** Shift from the special-condition PIA field to an integer PI level. */
#define DPY_SPEC_PI_SHIFT       3U
/** CONO bit 29: initialize/reset the display. */
#define DPY_CO_INIT             0000100UL

/** Successful display submission. */
#define DPY_E_OK                0
/** Another display word is still awaiting DONE. */
#define DPY_E_BUSY             -3

/**
 * @brief Submit one 36-bit Type 340 instruction word and wait for DONE.
 * @param word Two packed 18-bit Type 340 display instructions.
 * @return DPY_E_OK, or DPY_E_BUSY if another word is already in flight.
 */
int dpy_putword(kword_t word);

/** @brief Resident PI6 entry combining APR-clock and Type 340 service. */
void dpy_pi_handler(void);

#endif
