/* S6REC word reader used by shells, not INIT configuration parsing. */
#include "text.h"

#define U_S6REC_TYPE_SHIFT 30U
#define U_S6REC_TYPE_MASK  077UL
#define U_S6REC_TEXT       1U
#define U_S6REC_LEN_MASK   077777777UL

int
u_text_gets6(struct u_text_reader *r, kword_t *buf, unsigned int words)
{
        kword_t header;
        kword_t len;
        kword_t word;
        unsigned int payload;
        unsigned int i;
        int rc;

        if (r == 0 || r->fd < 0 || buf == 0 || words < 2U)
                return U_TEXT_ERROR;
        rc = u_text_next_word(r, &header);
        if (rc == 0)
                return U_TEXT_EOF;
        if (rc < 0)
                return U_TEXT_IOERROR;
        if ((unsigned int)((header >> U_S6REC_TYPE_SHIFT) &
            U_S6REC_TYPE_MASK) != U_S6REC_TEXT)
                return U_TEXT_ERROR;
        len = header & U_S6REC_LEN_MASK;
        payload = (unsigned int)((len + 5UL) / 6UL);
        if (payload + 1U > words)
                return U_TEXT_ERROR;
        buf[0] = len;
        for (i = 0U; i < payload; ++i) {
                rc = u_text_next_word(r, &word);
                if (rc < 0)
                        return U_TEXT_IOERROR;
                if (rc == 0)
                        return U_TEXT_ERROR;
                buf[i + 1U] = word;
        }
        for (i = payload + 1U; i < words; ++i)
                buf[i] = 0;
        return (int)len;
}
