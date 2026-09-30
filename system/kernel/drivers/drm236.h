/**
 * @file drm236.h
 * @brief PDP-6 Type 167 I/O Processor / Type 236 drum block interface.
 *
 * DAIMOS exposes each Type 236 drum as 8192 physical blocks of 128 36-bit
 * words. Up to four units are supported. KINIT probes the controller, installs
 * the resident DRM MRES, and patches the generic KCORE bridge to the relocated
 * read/write services. Early boot uses the same service entry points in polled
 * mode; after the process table exists, transfers use PI2 completion.
 */
#ifndef DAIMON_DRM236_H
#define DAIMON_DRM236_H

#include "kcore.h"

/** Number of supported Type 236 drum units. */
#define DRM236_UNITS             4U
/** DAIMOS physical block size in 36-bit words. */
#define DRM236_WORDS_PER_BLOCK   0200U
/** Number of 128-word blocks per drum unit. */
#define DRM236_BLOCKS_PER_UNIT   020000U
/** Native Type 236 address granularity: 16 words per hardware group. */
#define DRM236_GROUP_WORDS       020U
/** Number of 16-word groups in one DAIMOS 128-word block. */
#define DRM236_BLOCK_GROUPS      010U

/**
 * @brief Read one 128-word physical block.
 * @param unit Drum unit 0..3.
 * @param block Physical block 0..8191 within the selected unit.
 * @param buf Caller-owned 128-word destination buffer.
 * @return 0 on success, -1 for invalid arguments or I/O failure.
 */
int drm236_read_block(unsigned int unit, kword_t block, kword_t *buf);

/**
 * @brief Write one 128-word physical block.
 * @param unit Drum unit 0..3.
 * @param block Physical block 0..8191 within the selected unit.
 * @param buf Caller-owned 128-word source buffer.
 * @return 0 on success, -1 for invalid arguments or I/O failure.
 */
int drm236_write_block(unsigned int unit, kword_t block,
    const kword_t *buf);

#endif
