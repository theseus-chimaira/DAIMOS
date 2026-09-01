#ifndef DAIMON_D6LOG_V2_H
#define DAIMON_D6LOG_V2_H

#include "d6fs_disk.h"

#define D6LOG_V2_MAGIC          0442654574722UL /* SIXBIT /D6LOG2/ */
#define D6LOG_V2_RECORD_MAGIC   0442662454322UL /* SIXBIT /D6REC2/ */
#define D6LOG_V2_VERSION        2U
#define D6LOG_V2_RECORD_WORDS   8U
#define D6LOG_V2_PAYLOAD_WORDS  3U
#define D6LOG_V2_RECORDS_BLOCK  (D6FS_V2_BLOCK_WORDS / D6LOG_V2_RECORD_WORDS)

struct d6log {
        struct d6fs_dsk_v2 *disk;
        kword_t next_sequence;
        unsigned int next_slot;
        kword_t lost_records;
        unsigned int capacity;
};

int d6log_recover(struct d6log *log, struct d6fs_dsk_v2 *disk,
    kword_t scratch[D6FS_V2_BLOCK_WORDS]);
int d6log_append(struct d6log *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[D6FS_V2_BLOCK_WORDS]);

#endif
