#ifndef DAIMON_EXEC_H
#define DAIMON_EXEC_H

#include "proc.h"

#define EXEC_DXR_BASE_HDR_WORDS     2U
#define EXEC_DXR_EXT_HDR_WORDS      3U
#define EXEC_DXR_BSS_MASK           0077777U
#define EXEC_DXR_F_COMPRESSED       0100000U
#define EXEC_DXR_F_PURE             0200000U
#define EXEC_DXR_F_IMPURE           0400000U
#define EXEC_DXR_STACK_WORDS        02000U
#define EXEC_USER_ORIGIN            000020U
#define EXEC_DXR_MAX_IMAGE_WORDS    036000U
#define EXEC_DXR_MAX_BSS_WORDS      020000U
#define EXEC_DXR_TEXT_TAG            0647022U /* SIXBIT /TX2/ */

int exec_load_process(struct proc *p, unsigned int owner,
    const kword_t *path);
int exec_replace_current(const kword_t *block,
    unsigned int available_words, kword_t *entry_startup);

#endif
