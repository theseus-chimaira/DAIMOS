/**
 * @file kcore.h
 * @brief Common C definitions for the fixed resident kernel core.
 *
 * KCORE is the permanently resident low-memory portion of DAIMOS.  KINIT
 * installs its initialized image at the fixed PDP-6 address KCORE_BASE, clears
 * its BSS through __kcore_low_end, and then places MRES packages immediately
 * above that boundary.  Code including this header therefore refers to the
 * resident kernel address space, not to transient KINIT storage.
 *
 * The initial machine is a PDP-6, so kernel words are 36 bits wide and usable
 * addresses occupy an 18-bit halfword.  The cross compilers represent one
 * machine word with unsigned long; kword_t is the common source-level name
 * used where the exact target word representation matters.
 */
#ifndef DAIMON_KCORE_H
#define DAIMON_KCORE_H

#ifndef DAIMON_KWORD_T_DEFINED
#define DAIMON_KWORD_T_DEFINED
/** Unsigned representation of one PDP-6/PDP-10 36-bit machine word. */
typedef unsigned long kword_t;
#endif

/**
 * First word of the permanently resident KCORE image in PDP-6 low memory.
 *
 * Address 000060 immediately follows the architectural low-memory region used
 * by the PDP-6 and is also the fixed address of the shared D6LZ36 decoder.
 * The KCORE link therefore begins here and all resident-size accounting uses
 * this address as its lower bound.
 */
#define KCORE_BASE              000060UL

#endif
