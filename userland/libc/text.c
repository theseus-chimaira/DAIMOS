#include "text.h"

#define U_S6REC_TYPE_SHIFT 30U
#define U_S6REC_TYPE_MASK  077UL
#define U_S6REC_TEXT       1U
#define U_S6REC_LEN_MASK   077777777UL

int
u_text_open_fd(struct u_text_reader *r, int fd)
{
        if (r == 0 || fd < 0)
                return -1;
        r->fd = fd;
        r->pos = 0U;
        r->used = 0U;
        return 0;
}

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

static int
u_text_next_word(struct u_text_reader *r, kword_t *wordp)
{
        int rc;

        if (r->pos == r->used) {
                rc = dsys_read_words(r->fd, r->words, U_TEXT_READ_WORDS);
                if (rc <= 0)
                        return rc;
                r->pos = 0U;
                r->used = (unsigned int)rc;
        }
        *wordp = r->words[r->pos++];
        return 1;
}

int
u_text_getline(struct u_text_reader *r, char *buf, unsigned int size)
{
        kword_t header;
        kword_t word;
        kword_t len;
        unsigned int i;
        unsigned int words;
        unsigned int slot;
        unsigned int shift;
        int rc;

        if (r == 0 || r->fd < 0 || buf == 0 || size < 2U)
                return U_TEXT_ERROR;

        rc = u_text_next_word(r, &header);
        if (rc == 0)
                return U_TEXT_EOF;
        if (rc != 1 ||
            (unsigned int)((header >> U_S6REC_TYPE_SHIFT) &
            U_S6REC_TYPE_MASK) != U_S6REC_TEXT)
                return U_TEXT_ERROR;

        len = header & U_S6REC_LEN_MASK;
        words = (unsigned int)((len + 5UL) / 6UL);
        if (len + 1UL > (kword_t)size)
                return U_TEXT_ERROR;

        word = 0UL;
        slot = 6U;
        for (i = 0U; (kword_t)i < len; ++i) {
                if (slot == 6U) {
                        if (words == 0U ||
                            u_text_next_word(r, &word) != 1)
                                return U_TEXT_ERROR;
                        --words;
                        slot = 0U;
                }
                shift = 30U - slot * 6U;
                buf[i] = (char)(((word >> shift) & 077UL) + 040UL);
                ++slot;
        }
        while (words != 0U) {
                if (u_text_next_word(r, &word) != 1)
                        return U_TEXT_ERROR;
                --words;
        }
        buf[i] = 0;
        return (int)i;
}

void
u_text_close(struct u_text_reader *r)
{
        if (r != 0 && r->fd >= 0) {
                (void)dsys_close(r->fd);
                r->fd = -1;
                r->pos = 0U;
                r->used = 0U;
        }
}
