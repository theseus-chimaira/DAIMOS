#include "u.h"
#include "logevent.h"
#include "logstore.h"

#if LOGCOMPAT_PROGRAM == 2
static kword_t log_block[BLOCKSET_BLOCK_WORDS];

static int
log_record_valid(const kword_t *record)
{
        unsigned int payload;
        kword_t value;

        if (record[0] != LOGSTORE_RECORD_MAGIC || record[1] == 0UL)
                return 0;
        payload = (unsigned int)(record[3] & 0777777UL);
        if (payload > LOGSTORE_PAYLOAD_WORDS)
                return 0;
        value = (LOGSTORE_RECORD_MAGIC + record[1] +
            ((kword_t)payload << 18)) & LOGSTORE_WORD_MASK;
        return record[BLOCKSET_BLOCK_WORDS - 1U] ==
            ((value ^ LOGSTORE_WORD_MASK) | 1UL);
}

static int
log_print_record(const kword_t *record)
{
        unsigned int severity;
        unsigned int source;
        unsigned int payload;
        unsigned int i;

        severity = (unsigned int)((record[3] >> 30) & 077UL);
        source = (unsigned int)((record[3] >> 18) & 07777UL);
        payload = (unsigned int)(record[3] & 0777777UL);
        if (u_puts(1, "LOG SEQ ") != 0 || u_put_uint(1, record[1]) != 0 ||
            u_puts(1, " TIME ") != 0 || u_put_octal(1, record[2], 12U) != 0 ||
            u_puts(1, " SEV ") != 0 || u_put_uint(1, severity) != 0 ||
            u_puts(1, " SRC ") != 0 || u_put_uint(1, source) != 0)
                return 1;
        for (i = 0U; i < payload; ++i)
                if (u_putc(1, ' ') != 0 ||
                    u_put_octal(1, record[4U + i], 12U) != 0)
                        return 1;
        return u_crlf(1) != 0;
}
#endif

#if LOGCOMPAT_PROGRAM == 1
static int
logd_main(void)
{
        kword_t status[3];

        if (dsys_logctl(SYS_LOGCTL_STATUS, 0UL, status) != 0)
                return 1;
        (void)ulog_event(ULOG_SEV_INFO, ULOG_SRC_INIT, ULOG_INIT_READY, 0UL);
        return u_puts(1, "LOGSTORE READY") != 0 || u_crlf(1) != 0;
}
#endif

#if LOGCOMPAT_PROGRAM == 2
static int
logdump_main(void)
{
        kword_t status[3];
        kword_t next;
        kword_t oldest;
        kword_t seq;
        kword_t distance;
        unsigned int capacity;
        unsigned int next_slot;
        unsigned int slot;

        if (dsys_logctl(SYS_LOGCTL_STATUS, 0UL, status) != 0)
                return 1;
        next = status[0];
        capacity = (unsigned int)((status[1] >> 18) & 0777777UL);
        next_slot = (unsigned int)(status[1] & 0777777UL);
        if (next == 0UL || capacity == 0U || next_slot >= capacity)
                return 1;
        oldest = next <= 1UL ? 1UL : next - 1UL;
        if (oldest >= (kword_t)capacity)
                oldest = oldest - (kword_t)capacity + 1UL;
        else
                oldest = 1UL;
        for (seq = oldest; seq < next; ++seq) {
                distance = next - seq;
                slot = next_slot + capacity - (unsigned int)distance;
                if (slot >= capacity) slot -= capacity;
                if (dsys_logctl(SYS_LOGCTL_READ_BLOCK,
                    (kword_t)(slot + 2U), log_block) != 0)
                        return 1;
                if (!log_record_valid(log_block) || log_block[1] != seq)
                        continue;
                if (log_print_record(log_block) != 0)
                        return 1;
        }
        return 0;
}
#endif

int
main(int argc, kword_t **argv)
{
        (void)argv;
        if (argc != 1)
                return 2;
#if LOGCOMPAT_PROGRAM == 1
        return logd_main();
#elif LOGCOMPAT_PROGRAM == 2
        return logdump_main();
#else
        return 2;
#endif
}
