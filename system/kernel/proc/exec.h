/**
 * @file exec.h
 * @brief DAIMOS DXR executable format and resident EXEC loader interface.
 *
 * DXR images contain a two-word base header, optional TX2 extension word, image
 * payload, and a two-bit relocation map.  Compressed images require the TX2
 * extension; PURE images may retain file backing for swap/reload; RT_REQUIRED
 * reserves the single real-time owner slot while the image is active.
 */
#ifndef DAIMON_EXEC_H
#define DAIMON_EXEC_H

#include "proc.h"

#define EXEC_DXR_BASE_HDR_WORDS     2U
#define EXEC_DXR_EXT_HDR_WORDS      3U
#define EXEC_DXR_BSS_MASK           0077777U
#define EXEC_DXR_F_COMPRESSED       0100000U
#define EXEC_DXR_F_PURE             0200000U
#define EXEC_DXR_F_RT_REQUIRED      0400000U
#define EXEC_DXR_STACK_WORDS        02000U
#define EXEC_USER_ORIGIN            000020U
#define EXEC_DXR_MAX_IMAGE_WORDS    036000U
#define EXEC_DXR_MAX_BSS_WORDS      EXEC_DXR_BSS_MASK
#define EXEC_DXR_TEXT_TAG            0647022U /* SIXBIT /TX2/ */

#define EXEC_LOAD_OK                 0
#define EXEC_LOAD_RT_REQUIRED        1
#define EXEC_LOAD_NOMEM             -2
#define EXEC_REPLACE_FATAL          -2

/** Load and validate one DXR image into a process VM. */
int exec_load_process(struct proc *p, unsigned int owner,
    const kword_t *path);
/** Transactionally replace the current process image from an EXEC V1 block. */
int exec_replace_current(const kword_t *block,
    unsigned int available_words, kword_t *entry_startup);

#endif
