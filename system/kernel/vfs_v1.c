#include "vfs_v1.h"

extern int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);
