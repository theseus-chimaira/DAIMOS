/**
 * @file mres_reloc.h
 * @brief Two-bit relocation codes used by MRES images.
 *
 * Each initialized image word consumes one two-bit relocation code.  The code
 * says whether the installed image base must be applied to neither, either, or
 * both 18-bit halves of that word.
 */
#ifndef DAIMON_MRES_RELOC_H
#define DAIMON_MRES_RELOC_H

/** Word contains no relocatable 18-bit address. */
#define MRES_RELOC_NONE         0U
/** Relocate only the right 18-bit half. */
#define MRES_RELOC_RH18         1U
/** Relocate only the left 18-bit half. */
#define MRES_RELOC_LH18         2U
/** Relocate both 18-bit halves independently. */
#define MRES_RELOC_BOTH         3U

#endif
