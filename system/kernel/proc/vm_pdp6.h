#ifndef DAIMON_VM_PDP6_H
#define DAIMON_VM_PDP6_H

#include "vm.h"

#define VM_PDP6_ALIGN_WORDS VM_EXTENT_ALIGN_WORDS
#define VM_PDP6_BASE(p) ((kword_t)((p)->vm_state & PROC_HALF_MASK))
#define VM_PDP6_SET_BASE(p, b) \
        ((p)->vm_state = ((p)->vm_state & \
        ((kword_t)PROC_HALF_MASK << PROC_HALF_SHIFT)) | \
        ((kword_t)(b) & PROC_HALF_MASK))
#define VM_PDP6_SET_SPACE(p, words, base) \
        ((p)->vm_state = (((kword_t)(words) & PROC_HALF_MASK) << \
        PROC_HALF_SHIFT) | ((kword_t)(base) & PROC_HALF_MASK))

#endif
