#include "u.h"

struct libc_stat {
        long st_size;
        long st_mtime;
};

int
stat(char *path, struct libc_stat *out)
{
        kword_t packed[U_PATH_WORDS];
        struct vfs_stat st;

        if (path == 0 || out == 0 ||
            u_s6_pack(packed, U_PATH_WORDS, path) != 0 ||
            dsys_stat(packed, &st) != 0)
                return -1;
        out->st_size = (long)(st.size_words * sizeof(kword_t));
        out->st_mtime = (long)st.mtime;
        return 0;
}

long
time(long *result)
{
        long value;

        value = (long)dsys_gettime();
        if (result != 0)
                *result = value;
        return value;
}
