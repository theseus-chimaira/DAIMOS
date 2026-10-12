/* Path-based S6REC open is separate from fd-based reading so DSH
 * need not carry a redundant path packer or stat/open wrapper. */
#include "text.h"

int
u_text_open(struct u_text_reader *r, const char *path)
{
        kword_t packed[U_PATH_WORDS];
        struct vfs_stat st;
        int fd;

        if (r == 0 || path == 0)
                return -1;
        if (u_s6_pack(packed, U_PATH_WORDS, path) != 0)
                return -1;
        if (dsys_stat(packed, &st) != 0 || st.type != VFS_TYPE_REG)
                return -1;
        fd = dsys_open(packed, SYS_O_RDONLY);
        if (fd < 0)
                return -1;
        return u_text_open_fd(r, fd);
}
