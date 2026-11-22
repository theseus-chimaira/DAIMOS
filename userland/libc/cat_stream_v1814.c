#include "text.h"

/* CAT-only S6REC forwarding.  Other native commands must not pay for
 * this path in their resident TEXT image.  Keep it in a separate DARC
 * member which is extracted only when CAT requires u_text_copy_line. */
#define U_S6REC_TYPE_SHIFT 30U
#define U_S6REC_TYPE_MASK  077UL
#define U_S6REC_TEXT       1U
#define U_S6REC_LEN_MASK   077777777UL

static int
cat_next_word(struct u_text_reader *r, kword_t *wordp)
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

/* Forward one complete S6REC text record without a line-sized buffer.
 * Files and pipes are native word streams, so copy the original header
 * and data in chunks from the existing reader buffer.  TTY output must
 * use WRITECHAR: its WRITE_WORDS interface accepts a whole record in
 * one call, while the bounded u_text_sink cannot hold long source lines.
 * This preserves long lines, native S6REC file format and low peak RAM. */
int
u_text_copy_line(struct u_text_reader *r, int out_fd)
{
        kword_t header;
        kword_t word;
        kword_t remaining;
        int tty;
        unsigned int i;
        unsigned int count;
        unsigned int shift;
        int rc;

        if (r == 0 || r->fd < 0 || out_fd < 0)
                return U_TEXT_ERROR;
        rc = cat_next_word(r, &header);
        if (rc == 0)
                return U_TEXT_EOF;
        if (rc != 1 ||
            (unsigned int)((header >> U_S6REC_TYPE_SHIFT) &
            U_S6REC_TYPE_MASK) != U_S6REC_TEXT)
                return U_TEXT_ERROR;

        remaining = header & U_S6REC_LEN_MASK;
        tty = dsys_isatty(out_fd) >= 0;
        if (!tty) {
                remaining = (remaining + 5UL) / 6UL;
                if (u_write_words_all(out_fd, &header, 1U) != 0)
                        return U_TEXT_ERROR;
                while (remaining != 0UL) {
                        if (r->pos == r->used) {
                                rc = dsys_read_words(r->fd, r->words,
                                    U_TEXT_READ_WORDS);
                                if (rc <= 0)
                                        return U_TEXT_ERROR;
                                r->pos = 0U;
                                r->used = (unsigned int)rc;
                        }
                        count = r->used - r->pos;
                        if ((kword_t)count > remaining)
                                count = (unsigned int)remaining;
                        if (u_write_words_all(out_fd,
                            &r->words[r->pos], count) != 0)
                                return U_TEXT_ERROR;
                        r->pos += count;
                        remaining -= (kword_t)count;
                }
                return 1;
        }

        while (remaining != 0UL) {
                if (cat_next_word(r, &word) != 1)
                        return U_TEXT_ERROR;
                count = remaining > 6UL ? 6U : (unsigned int)remaining;
                for (i = 0U; i < count; ++i) {
                        shift = 30U - i * 6U;
                        if (u_putc(out_fd,
                                (int)(((word >> shift) & 077UL) + 040UL)) != 0)
                                return U_TEXT_ERROR;
                }
                remaining -= (kword_t)count;
        }
        return u_crlf(out_fd) == 0 ? 1 : U_TEXT_ERROR;
}
