#ifndef DAIMON_LOGSTORE_H
#define DAIMON_LOGSTORE_H

#include "diskset.h"

#define LOGSTORE_WORD_MASK      0777777777777UL
#define LOGSTORE_RECORD_MAGIC   0546362454321UL /* SIXBIT /LSREC1/ */
#define LOGSTORE_STATE_MAGIC    0546363644121UL /* SIXBIT /LSSTA1/ */
#define LOGSTORE_PAYLOAD_WORDS  (DISKSET_BLOCK_WORDS - 5U)
#define LOGSTORE_STATE_NONE     2U

#define LOGSTORE_DRAIN_OK       0
#define LOGSTORE_DRAIN_EMPTY    1
#define LOGSTORE_DRAIN_REEL     2

struct logstore {
        kword_t next_sequence;
        kword_t next_drain_sequence;
        kword_t lost_records;
        kword_t state_generation;
        unsigned int next_slot;
        unsigned int capacity;
        unsigned int state_copy;
};

int logstore_recover(struct logstore *log,
    kword_t scratch[DISKSET_BLOCK_WORDS]);
int logstore_append(struct logstore *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[DISKSET_BLOCK_WORDS]);

/* Drain support is kept outside the boot image and owns no transfer buffer. */
int logstore_drain_one(struct logstore *log, unsigned int unit,
    kword_t scratch[DISKSET_BLOCK_WORDS]);

/* Shared with the cold drain path; not a general LOGSTORE interface. */
int logstore_record_valid(const kword_t block[DISKSET_BLOCK_WORDS]);
int logstore_write_state(struct logstore *log, kword_t next_drain_sequence,
    kword_t lost_records, kword_t scratch[DISKSET_BLOCK_WORDS]);

#endif
