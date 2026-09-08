#ifndef DAIMON_EXEC_H
#define DAIMON_EXEC_H

#include "proc.h"

#define EXEC_DXR_HDR_WORDS          2U
#define EXEC_DXR_BSS_MASK           0177777U
#define EXEC_DXR_F_PURE             0200000U
#define EXEC_DXR_F_IMPURE           0400000U
#define EXEC_DXR_STACK_WORDS        02000U
#define EXEC_DXR_MAX_IMAGE_WORDS    036000U
#define EXEC_DXR_MAX_BSS_WORDS      020000U
#define EXEC_SYSCALL_MARKER         0777777777777UL
#define EXEC_PDP10_JRST             0254000000000UL

int exec_load_init(struct proc *p, unsigned int owner,
    const kword_t *path, kword_t base, kword_t limit);

#endif
