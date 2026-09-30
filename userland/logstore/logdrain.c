#include "u.h"
#include "logstore.h"

#define LOGDRAIN_FILE_RECORDS 128U
#define LOGDRAIN_MTC_REEL_MASK 0600000UL

#define SINK_NULL    0U
#define SINK_FILE    1U
#define SINK_CONSOLE 2U
#define SINK_MTC     3U

struct drain_state {
        kword_t next_sequence;
        kword_t lost_records;
        kword_t generation;
        unsigned int copy;
};

struct producer_state {
        kword_t next_sequence;
        unsigned int next_slot;
        unsigned int capacity;
        kword_t blocks;
};

struct sink_state {
        unsigned int kind;
        unsigned int unit;
        unsigned int severity;
        int fd;
        kword_t file_words;
        kword_t path[U_PATH_WORDS];
        kword_t oldpath[U_PATH_WORDS];
};

static kword_t block[BLOCKSET_BLOCK_WORDS];

static void
zero_block(void)
{
        unsigned int i;
        for (i = 0U; i < BLOCKSET_BLOCK_WORDS; ++i)
                block[i] = 0UL;
}

static kword_t
state_commit(kword_t generation, kword_t next_sequence,
    kword_t lost_records, unsigned int capacity)
{
        kword_t v;
        v = (LOGSTORE_STATE_MAGIC + generation + next_sequence +
            lost_records + (kword_t)capacity) & LOGSTORE_WORD_MASK;
        return (v ^ LOGSTORE_WORD_MASK) | 1UL;
}

static int
state_valid(const kword_t *b, unsigned int capacity)
{
        return b[0] == LOGSTORE_STATE_MAGIC && b[1] != 0UL &&
            b[2] != 0UL && b[4] == (kword_t)capacity &&
            b[BLOCKSET_BLOCK_WORDS - 1U] ==
            state_commit(b[1], b[2], b[3], capacity);
}

static kword_t
record_commit(kword_t sequence, unsigned int payload_words)
{
        kword_t v;
        v = (LOGSTORE_RECORD_MAGIC + sequence +
            ((kword_t)payload_words << 18)) & LOGSTORE_WORD_MASK;
        return (v ^ LOGSTORE_WORD_MASK) | 1UL;
}

static int
record_valid(const kword_t *b)
{
        unsigned int words;
        if (b[0] != LOGSTORE_RECORD_MAGIC || b[1] == 0UL)
                return 0;
        words = (unsigned int)(b[3] & 0777777UL);
        return words <= LOGSTORE_PAYLOAD_WORDS &&
            b[BLOCKSET_BLOCK_WORDS - 1U] == record_commit(b[1], words);
}

static int
producer_status(struct producer_state *p)
{
        kword_t status[3];
        if (p == 0 || dsys_logctl(SYS_LOGCTL_STATUS, 0UL, status) != 0)
                return -1;
        p->next_sequence = status[0];
        p->capacity = (unsigned int)((status[1] >> 18) & 0777777UL);
        p->next_slot = (unsigned int)(status[1] & 0777777UL);
        p->blocks = status[2];
        if (p->next_sequence == 0UL || p->capacity == 0U ||
            p->next_slot >= p->capacity ||
            p->blocks != (kword_t)p->capacity + 2UL)
                return -1;
        return 0;
}

static kword_t
oldest_sequence(const struct producer_state *p)
{
        kword_t newest;
        if (p->next_sequence <= 1UL)
                return 1UL;
        newest = p->next_sequence - 1UL;
        if (newest >= (kword_t)p->capacity)
                return newest - (kword_t)p->capacity + 1UL;
        return 1UL;
}

static int
drain_recover(const struct producer_state *p, struct drain_state *d)
{
        kword_t best_generation;
        kword_t oldest;
        unsigned int slot;
        int have;

        if (p == 0 || d == 0)
                return -1;
        best_generation = 0UL;
        have = 0;
        d->next_sequence = 1UL;
        d->lost_records = 0UL;
        d->generation = 0UL;
        d->copy = LOGSTORE_STATE_NONE;
        for (slot = 0U; slot < 2U; ++slot) {
                if (dsys_logctl(SYS_LOGCTL_READ_BLOCK, (kword_t)slot,
                    block) != 0)
                        return -1;
                if (state_valid(block, p->capacity) &&
                    (!have || block[1] > best_generation)) {
                        have = 1;
                        best_generation = block[1];
                        d->generation = block[1];
                        d->next_sequence = block[2];
                        d->lost_records = block[3];
                        d->copy = slot;
                }
        }
        oldest = oldest_sequence(p);
        if (!have || d->next_sequence > p->next_sequence)
                d->next_sequence = oldest;
        return 0;
}

static int
drain_write_state(const struct producer_state *p,
    struct drain_state *d, kword_t next_sequence, kword_t lost_records)
{
        kword_t generation;
        unsigned int copy;

        if (p == 0 || d == 0 || next_sequence == 0UL ||
            d->generation == LOGSTORE_WORD_MASK)
                return -1;
        generation = d->generation + 1UL;
        copy = d->copy <= 1U ? d->copy ^ 1U : 0U;
        zero_block();
        block[0] = LOGSTORE_STATE_MAGIC;
        block[1] = generation;
        block[2] = next_sequence;
        block[3] = lost_records;
        block[4] = (kword_t)p->capacity;
        block[BLOCKSET_BLOCK_WORDS - 1U] =
            state_commit(generation, next_sequence, lost_records, p->capacity);
        if (dsys_logctl(SYS_LOGCTL_WRITE_BLOCK, (kword_t)copy, block) != 0)
                return -1;
        d->generation = generation;
        d->copy = copy;
        d->next_sequence = next_sequence;
        d->lost_records = lost_records;
        return 0;
}

static int
parse_uint(const kword_t *arg, unsigned int max, unsigned int *out)
{
        unsigned int n;
        unsigned int i;
        unsigned int wi;
        unsigned int sh;
        unsigned int ch;
        unsigned int v;

        if (arg == 0 || out == 0)
                return -1;
        n = (unsigned int)(arg[0] & 0777777UL);
        if (n == 0U)
                return -1;
        v = 0U;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((arg[wi] >> sh) & 077UL) + 040U);
                if (ch < '0' || ch > '9')
                        return -1;
                if (v > (max - (ch - '0')) / 10U)
                        return -1;
                v = v * 10U + ch - '0';
        }
        *out = v;
        return 0;
}

static int
sink_file_open(struct sink_state *s)
{
        struct vfs_stat st;

        if (dsys_stat(s->path, &st) == 0) {
                if (st.type != VFS_TYPE_REG)
                        return -1;
                if (st.size_words >=
                    (kword_t)LOGDRAIN_FILE_RECORDS * BLOCKSET_BLOCK_WORDS) {
                        (void)dsys_unlink(s->oldpath);
                        if (dsys_rename(s->path, s->oldpath) != 0)
                                return -1;
                        s->file_words = 0UL;
                } else {
                        s->file_words = st.size_words;
                }
        } else {
                s->file_words = 0UL;
        }
        s->fd = dsys_open(s->path,
            SYS_O_WRONLY | SYS_O_CREAT | SYS_O_APPEND);
        return s->fd < 0 ? -1 : 0;
}

static int
console_record(const kword_t *record, unsigned int threshold)
{
        unsigned int severity;
        unsigned int source;
        unsigned int payload;
        unsigned int i;

        severity = (unsigned int)((record[3] >> 30) & 077UL);
        if (severity < threshold)
                return 0;
        source = (unsigned int)((record[3] >> 18) & 07777UL);
        payload = (unsigned int)(record[3] & 0777777UL);
        if (u_puts(1, "LOG SEQ ") != 0 ||
            u_put_uint(1, record[1]) != 0 ||
            u_puts(1, " TIME ") != 0 ||
            u_put_octal(1, record[2], 12U) != 0 ||
            u_puts(1, " SEV ") != 0 ||
            u_put_uint(1, (kword_t)severity) != 0 ||
            u_puts(1, " SRC ") != 0 ||
            u_put_uint(1, (kword_t)source) != 0)
                return -1;
        for (i = 0U; i < payload; ++i) {
                if (u_putc(1, ' ') != 0 ||
                    u_put_octal(1, record[4U + i], 12U) != 0)
                        return -1;
        }
        return u_crlf(1) != 0 ? -1 : 0;
}

static int
sink_record(struct sink_state *s, const kword_t *record)
{
        kword_t status;
        int n;

        if (s->kind == SINK_NULL)
                return 0;
        if (s->kind == SINK_CONSOLE)
                return console_record(record, s->severity);
        if (s->kind == SINK_FILE) {
                n = dsys_write_words(s->fd, (kword_t *)record,
                    BLOCKSET_BLOCK_WORDS);
                if (n != (int)BLOCKSET_BLOCK_WORDS)
                        return -1;
                s->file_words += BLOCKSET_BLOCK_WORDS;
                return 0;
        }
        if (s->kind == SINK_MTC) {
                status = 0UL;
                if (dsys_logctl(SYS_LOGCTL_MTC_STATUS,
                    (kword_t)s->unit, &status) != 0)
                        return -1;
                if ((status & LOGDRAIN_MTC_REEL_MASK) != 0UL)
                        return LOGSTORE_DRAIN_REEL;
                if (dsys_logctl(SYS_LOGCTL_MTC_WRITE,
                    (kword_t)s->unit, (kword_t *)record) != 0)
                        return -1;
                return 0;
        }
        return -1;
}

static int
report_gap(kword_t lost)
{
        return u_puts(2, "LOGDRAIN: LOST ") != 0 ||
            u_put_uint(2, lost) != 0 || u_crlf(2) != 0 ? -1 : 0;
}

static int
drain_one(struct sink_state *sink, struct producer_state *p,
    struct drain_state *d)
{
        kword_t oldest;
        kword_t lost;
        kword_t distance;
        unsigned int slot;
        int rc;

        if (producer_status(p) != 0)
                return -1;
        oldest = oldest_sequence(p);
        if (d->next_sequence < oldest) {
                lost = oldest - d->next_sequence;
                if (lost > LOGSTORE_WORD_MASK - d->lost_records)
                        return -1;
                if (drain_write_state(p, d, oldest,
                    d->lost_records + lost) != 0)
                        return -1;
                if (report_gap(lost) != 0)
                        return -1;
        }
        if (d->next_sequence == p->next_sequence)
                return LOGSTORE_DRAIN_EMPTY;
        if (d->next_sequence > p->next_sequence)
                return -1;
        distance = p->next_sequence - d->next_sequence;
        slot = p->next_slot + p->capacity - (unsigned int)distance;
        if (slot >= p->capacity)
                slot -= p->capacity;
        if (dsys_logctl(SYS_LOGCTL_READ_BLOCK, (kword_t)(slot + 2U),
            block) != 0 || !record_valid(block) ||
            block[1] != d->next_sequence)
                return -1;
        rc = sink_record(sink, block);
        if (rc != 0)
                return rc;
        return drain_write_state(p, d, d->next_sequence + 1UL,
            d->lost_records);
}

static int
setup_file_sink(struct sink_state *s)
{
        kword_t logdir[U_PATH_WORDS];

        if (u_s6_pack(logdir, U_PATH_WORDS, "/LOG") != 0 ||
            u_s6_pack(s->path, U_PATH_WORDS, "/LOG/SYSTEM.LOG") != 0 ||
            u_s6_pack(s->oldpath, U_PATH_WORDS, "/LOG/SYSTEM.OLD") != 0)
                return -1;
        (void)dsys_mkdir(logdir);
        return sink_file_open(s);
}

static int
setup_sink(int argc, kword_t **argv, int first, struct sink_state *s)
{
        unsigned int n;

        s->fd = -1;
        s->unit = 0U;
        s->severity = 0U;
        if (first >= argc)
                return -1;
        if (u_s6_eq(argv[first], "NULL")) {
                s->kind = SINK_NULL;
                return first + 1 == argc ? 0 : -1;
        }
        if (u_s6_eq(argv[first], "FILE")) {
                s->kind = SINK_FILE;
                return first + 1 == argc ? setup_file_sink(s) : -1;
        }
        if (u_s6_eq(argv[first], "CONSOLE")) {
                s->kind = SINK_CONSOLE;
                if (first + 1 == argc)
                        return 0;
                if (first + 2 != argc ||
                    parse_uint(argv[first + 1], 077U, &n) != 0)
                        return -1;
                s->severity = n;
                return 0;
        }
        if (u_s6_eq(argv[first], "MTC")) {
                s->kind = SINK_MTC;
                if (first + 1 == argc)
                        return 0;
                if (first + 2 != argc ||
                    parse_uint(argv[first + 1], 7U, &n) != 0)
                        return -1;
                s->unit = n;
                return 0;
        }
        return -1;
}

static int
show_status(void)
{
        struct producer_state p;
        struct drain_state d;

        if (producer_status(&p) != 0 || drain_recover(&p, &d) != 0)
                return -1;
        if (u_puts(1, "NEXT ") != 0 || u_put_uint(1, p.next_sequence) != 0 ||
            u_puts(1, " DRAIN ") != 0 || u_put_uint(1, d.next_sequence) != 0 ||
            u_puts(1, " LOST ") != 0 || u_put_uint(1, d.lost_records) != 0 ||
            u_puts(1, " CAP ") != 0 || u_put_uint(1, p.capacity) != 0 ||
            u_crlf(1) != 0)
                return -1;
        return 0;
}

static int
usage(void)
{
        (void)u_puts(2,
            "USAGE: LOGDRAIN [FOLLOW] NULL|FILE|CONSOLE [SEVERITY]|MTC [UNIT]");
        (void)u_crlf(2);
        (void)u_puts(2, "       LOGDRAIN STATUS");
        (void)u_crlf(2);
        return 1;
}

int
main(int argc, kword_t **argv)
{
        struct producer_state producer;
        struct drain_state drain;
        struct sink_state sink;
        int first;
        int follow;
        int rc;

        if (argc == 2 && u_s6_eq(argv[1], "STATUS"))
                return show_status() == 0 ? 0 : 1;
        follow = 0;
        first = 1;
        if (argc > 1 && u_s6_eq(argv[1], "FOLLOW")) {
                follow = 1;
                first = 2;
        }
        if (setup_sink(argc, argv, first, &sink) != 0)
                return usage();
        if (producer_status(&producer) != 0 ||
            drain_recover(&producer, &drain) != 0)
                return 1;
        for (;;) {
                rc = drain_one(&sink, &producer, &drain);
                if (rc == LOGSTORE_DRAIN_OK)
                        continue;
                if (rc == LOGSTORE_DRAIN_REEL) {
                        (void)u_puts(2, "LOGDRAIN: CHANGE REEL");
                        (void)u_crlf(2);
                        rc = 2;
                        break;
                }
                if (rc != LOGSTORE_DRAIN_EMPTY) {
                        rc = 1;
                        break;
                }
                if (!follow) {
                        rc = 0;
                        break;
                }
                if (dsys_logctl(SYS_LOGCTL_WAIT,
                    producer.next_sequence, 0) != 0) {
                        rc = 1;
                        break;
                }
        }
        if (sink.kind == SINK_MTC && rc == 0 && !follow) {
                if (dsys_logctl(SYS_LOGCTL_MTC_FILEMARK,
                    (kword_t)sink.unit, 0) != 0)
                        rc = 1;
        }
        if (sink.fd >= 0 && dsys_close(sink.fd) != 0)
                rc = 1;
        return rc;
}
