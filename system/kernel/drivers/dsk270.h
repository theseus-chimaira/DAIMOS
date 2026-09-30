/**
 * @file dsk270.h
 * @brief PDP-6 Type 270 fixed-disk physical-sector interface.
 *
 * DAIMOS exposes each Type 270 unit as 45056 physical sectors of 128 36-bit
 * words. Up to four units are supported. KINIT probes the shared Type 136/270
 * path, installs the resident DSK MRES, and patches the generic KCORE bridge to
 * the relocated read/write services.
 *
 * The raw driver owns physical addressing, one-sector transfer, controller
 * errors, a bounded per-unit elevator queue, and the active-transfer watchdog.
 * Filesystem/blockset mapping and persistent bad-media remapping remain above
 * this interface.
 */
#ifndef DAIMON_DSK270_H
#define DAIMON_DSK270_H

#include "kcore.h"
#include "storage.h"

/** Maximum number of supported Type 270 disk units. */
#define DSK270_UNITS             4U
/** Physical sector size in 36-bit words. */
#define DSK270_WORDS_PER_SECTOR  BSTORE_BLOCK_WORDS
/** Sectors per cylinder: octal 054 = decimal 44. */
#define DSK270_SECTORS_PER_CYL   054U
/** Cylinders per unit: octal 02000 = decimal 1024. */
#define DSK270_CYLINDERS         02000U
/** Total sectors per unit: octal 0130000 = decimal 45056. */
#define DSK270_SECTORS_PER_UNIT  0130000UL

/** Raw Type 270 address shift for the two-bit unit field. */
#define DSK270_HW_UNIT_SHIFT     16U
/** Raw Type 270 address shift for the ten-bit cylinder field. */
#define DSK270_CYL_SHIFT          6U

/**
 * @brief Read one physical Type 270 sector.
 * @param unit Disk unit 0..3.
 * @param sector Physical sector 0..45055 within that unit.
 * @param buf Caller-owned 128-word destination buffer.
 * @return 0 on success, negative storage status on validation/I/O failure.
 */
int dsk270_read_sector(unsigned int unit, kword_t sector, kword_t *buf);

/**
 * @brief Write one physical Type 270 sector.
 * @param unit Disk unit 0..3.
 * @param sector Physical sector 0..45055 within that unit.
 * @param buf Caller-owned 128-word source buffer.
 * @return 0 on success, negative storage status on validation/I/O failure.
 */
int dsk270_write_sector(unsigned int unit, kword_t sector,
    const kword_t *buf);


#endif
