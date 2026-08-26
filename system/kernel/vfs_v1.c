#include "vfs_v1.h"

int
vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        unsigned int i;

        if (name == 0 || chars > 6U)
                return -1;
        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        return 0;
}

int
vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        if (name == 0)
                return 0;
        return name->chars == chars && name->words[0] == word;
}
