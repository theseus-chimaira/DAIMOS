#ifndef DAIMON_EXEC_V1_H
#define DAIMON_EXEC_V1_H

#include "proc_v1.h"

#define EXEC_V1_DXR_HDR_WORDS          2U
#define EXEC_V1_DXR_STACK_WORDS        02000U
#define EXEC_V1_DXR_MAX_IMAGE_WORDS    036000U
#define EXEC_V1_DXR_MAX_BSS_WORDS      020000U
#define EXEC_V1_SYSCALL_MARKER         0777777777777UL
#define EXEC_V1_PDP10_JRST             0254000000000UL

int exec_v1_load_init(struct proc_v1 *p, unsigned int owner,
    const kword_t *path, kword_t base, kword_t limit);

#endif
