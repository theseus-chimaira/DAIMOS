#include "logevent.h"
#include "logstore.h"

#if !defined(DAIMOS_ULOG_STACK_RECORD) && !defined(DAIMOS_ULOG_COMPACT_EVENT)
static kword_t ulog_record[BLOCKSET_BLOCK_WORDS];
#endif

int
ulog_event(unsigned int severity, unsigned int source,
    kword_t code, kword_t value)
{
#ifdef DAIMOS_ULOG_COMPACT_EVENT
        /* PID 1 uses the bounded four-word event transport. LOGSTORE
         * constructs the full on-disk record in temporary dynamic core. */
        kword_t event[4];
#elif defined(DAIMOS_ULOG_STACK_RECORD)
        /* PID 1 serializes log calls. Borrow the already reserved user
         * stack rather than pinning a block-sized record in static BSS. */
        kword_t ulog_record[BLOCKSET_BLOCK_WORDS];
#endif
#ifndef DAIMOS_ULOG_COMPACT_EVENT
        unsigned int i;
#endif

        if (severity > 077U || source > 07777U)
                return -1;
#ifdef DAIMOS_ULOG_COMPACT_EVENT
        event[0] = dsys_gettime() & LOGSTORE_WORD_MASK;
        event[1] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | 2UL;
        event[2] = code & LOGSTORE_WORD_MASK;
        event[3] = value & LOGSTORE_WORD_MASK;
        return dsys_logctl(SYS_LOGCTL_EVENT, 0UL, event);
#else
#ifndef DAIMOS_ULOG_STATIC_REUSE
        for (i = 0U; i < BLOCKSET_BLOCK_WORDS; ++i)
                ulog_record[i] = 0UL;
#endif
        ulog_record[2] = dsys_gettime() & LOGSTORE_WORD_MASK;
        ulog_record[3] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | 2UL;
        ulog_record[4] = code & LOGSTORE_WORD_MASK;
        ulog_record[5] = value & LOGSTORE_WORD_MASK;
        return dsys_logctl(SYS_LOGCTL_APPEND, 0UL, ulog_record);
#endif
}
