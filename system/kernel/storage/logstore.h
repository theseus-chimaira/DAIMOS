#ifndef DAIMON_LOGSTORE_H
#define DAIMON_LOGSTORE_H

#include "blockset.h"

#define LOGSTORE_WORD_MASK      0777777777777UL
#define LOGSTORE_RECORD_MAGIC   0546362454321UL /* SIXBIT /LSREC1/ */
#define LOGSTORE_STATE_MAGIC    0546363644121UL /* SIXBIT /LSSTA1/ */
#define LOGSTORE_PAYLOAD_WORDS  (BLOCKSET_BLOCK_WORDS - 5U)
#define LOGSTORE_STATE_NONE     2U

#define LOGSTORE_DRAIN_OK       0
#define LOGSTORE_DRAIN_EMPTY    1
#define LOGSTORE_DRAIN_REEL     2

struct logstore {
        kword_t next_sequence;
        unsigned int next_slot;
        unsigned int capacity;
};

/* Optional consumer state.  The producer never depends on this structure. */
struct logstore_drain {
        kword_t next_sequence;
        kword_t lost_records;
        kword_t state_generation;
        unsigned int state_copy;
};

void logstore_boot_configure(kword_t start, kword_t blocks);
kword_t logstore_boot_blocks(void);
int logstore_boot_read(kword_t blockno, kword_t block[BLOCKSET_BLOCK_WORDS]);
int logstore_boot_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS]);

typedef int (*logstore_sink_fn)(void *context,
    const kword_t record[BLOCKSET_BLOCK_WORDS], unsigned int words);

int logstore_recover(struct logstore *log,
    kword_t scratch[BLOCKSET_BLOCK_WORDS]);
int logstore_append(struct logstore *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[BLOCKSET_BLOCK_WORDS]);
int logstore_record_valid(const kword_t block[BLOCKSET_BLOCK_WORDS]);

/* Optional drain support.  Not part of the disk-only LOGSTORE producer. */
int logstore_drain_recover(const struct logstore *log,
    struct logstore_drain *drain, kword_t scratch[BLOCKSET_BLOCK_WORDS]);
int logstore_drain_one(const struct logstore *log,
    struct logstore_drain *drain, logstore_sink_fn sink, void *context,
    kword_t scratch[BLOCKSET_BLOCK_WORDS]);

/* Optional magnetic-tape sink adapter. */
int logstore_mtc_sink(void *context,
    const kword_t record[BLOCKSET_BLOCK_WORDS], unsigned int words);

#endif
