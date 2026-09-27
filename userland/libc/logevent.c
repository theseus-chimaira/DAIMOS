#include "logevent.h"
#include "logstore.h"

static kword_t ulog_record[BLOCKSET_BLOCK_WORDS];

int
ulog_event(unsigned int severity, unsigned int source,
    kword_t code, kword_t value)
{
        unsigned int i;

        if (severity > 077U || source > 07777U)
                return -1;
        for (i = 0U; i < BLOCKSET_BLOCK_WORDS; ++i)
                ulog_record[i] = 0UL;
        ulog_record[2] = dsys_gettime() & LOGSTORE_WORD_MASK;
        ulog_record[3] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | 2UL;
        ulog_record[4] = code & LOGSTORE_WORD_MASK;
        ulog_record[5] = value & LOGSTORE_WORD_MASK;
        return dsys_logctl(SYS_LOGCTL_APPEND, 0UL, ulog_record);
}
