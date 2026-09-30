/**
 * @file mres.h
 * @brief Boot MRES package format and KINIT installation interface.
 *
 * An MRES package contains a compact three-word header followed by export
 * metadata, the resident image, and relocation information.  KINIT copies the
 * selected image into packed low managed core, applies relocation, publishes
 * exports, then commits the occupied range through MM as permanent resident
 * memory.
 */
#ifndef DAIMON_MRES_H
#define DAIMON_MRES_H

#include "kinit.h"
#include "mres_reloc.h"

/** SIXBIT package signature for the current MRES format. */
#define MRES_MAGIC              SIXBIT("MRES1 ")
/** Number of fixed words preceding package-specific tables/image data. */
#define MRES_HEADER_WORDS       3U

extern kword_t __kcore_load_begin;
extern kword_t __kcore_low_init_end;
extern kword_t __kcore_low_end;
extern unsigned int mres_last_owner;
extern unsigned int mres_last_image_words;

/** Copy the linked low KCORE image into its runtime physical location. */
void kcore_load(void);
/** Reset boot MRES owner/allocation state before module installation. */
void mres_init(void);
/** Install and relocate one MRES package; return its physical base. */
int mres_install(const kword_t *package, unsigned int *basep);
/** Resolve one package export index to its installed absolute address. */
unsigned int mres_export(const kword_t *package, unsigned int base,
    unsigned int index);
/** Invoke an installed MRES entry using the standard request ABI. */
int mres_call(unsigned int address, void *request);

#endif
