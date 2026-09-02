#include "d6log.h"
#include "d6fs_boot.h"

#define D6LOG_WORD_MASK      0777777777777UL

static int
d6log_header_valid(const kword_t *block, unsigned int capacity)
{
        return block[0] == D6LOG_MAGIC && block[1] == D6LOG_VERSION &&
            block[5] == (kword_t)capacity &&
            block[6] == D6LOG_RECORD_WORDS &&
            block[7] == D6LOG_PAYLOAD_WORDS;
}

static int
d6log_record_valid(const kword_t *record)
{
        return record[0] == D6LOG_RECORD_MAGIC && record[1] != 0UL &&
            (record[3] & 07UL) <= D6LOG_PAYLOAD_WORDS &&
            record[7] == ((D6LOG_RECORD_MAGIC ^ record[1]) &
            D6LOG_WORD_MASK);
}

static int
d6log_write_header(struct d6log *log,
    kword_t scratch[D6FS_BLOCK_WORDS])
{
        unsigned int i;

        for (i = 0U; i < D6FS_BLOCK_WORDS; ++i)
                scratch[i] = 0UL;
        scratch[0] = D6LOG_MAGIC;
        scratch[1] = D6LOG_VERSION;
        scratch[2] = log->next_sequence;
        scratch[3] = log->next_slot;
        scratch[4] = log->lost_records;
        scratch[5] = log->capacity;
        scratch[6] = D6LOG_RECORD_WORDS;
        scratch[7] = D6LOG_PAYLOAD_WORDS;
        return d6fs_boot_log_write(log->disk, 0UL, scratch);
}

int
d6log_recover(struct d6log *log, struct d6fs_dsk *disk,
    kword_t scratch[D6FS_BLOCK_WORDS])
{
        kword_t best_sequence;
        kword_t header_lost;
        unsigned int capacity;
        unsigned int slot;
        unsigned int loaded_block;
        unsigned int current_block;
        int have_header;

        if (log == 0 || disk == 0 || scratch == 0 || disk->logstore_blocks < 2UL)
                return -1;
        capacity = (unsigned int)((disk->logstore_blocks - 1UL) *
            D6LOG_RECORDS_BLOCK);
        if (capacity == 0U)
                return -1;
        if (d6fs_boot_log_read(disk, 0UL, scratch) != 0)
                return -1;
        have_header = d6log_header_valid(scratch, capacity);
        header_lost = have_header ? scratch[4] : 0UL;
        best_sequence = 0UL;
        loaded_block = ~0U;
        for (slot = 0U; slot < capacity; ++slot) {
                const kword_t *record;
                unsigned int offset;

                current_block = 1U + slot / D6LOG_RECORDS_BLOCK;
                if (current_block != loaded_block) {
                        if (d6fs_boot_log_read(disk,
                            (kword_t)current_block, scratch) != 0)
                                return -1;
                        loaded_block = current_block;
                }
                offset = (slot % D6LOG_RECORDS_BLOCK) *
                    D6LOG_RECORD_WORDS;
                record = scratch + offset;
                if (d6log_record_valid(record) && record[1] > best_sequence)
                        best_sequence = record[1];
        }
        log->disk = disk;
        log->capacity = capacity;
        if (best_sequence == 0UL) {
                log->next_sequence = 1UL;
                log->next_slot = 0U;
                log->lost_records = 0UL;
        } else {
                log->next_sequence = (best_sequence + 1UL) & D6LOG_WORD_MASK;
                if (log->next_sequence == 0UL)
                        log->next_sequence = 1UL;
                log->next_slot = (unsigned int)(best_sequence % capacity);
                if (best_sequence > (kword_t)capacity)
                        log->lost_records = best_sequence - (kword_t)capacity;
                else
                        log->lost_records = 0UL;
                if (header_lost > log->lost_records)
                        log->lost_records = header_lost;
        }
        return 0;
}

int
d6log_append(struct d6log *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[D6FS_BLOCK_WORDS])
{
        unsigned int blockno;
        unsigned int offset;
        unsigned int i;
        kword_t sequence;

        if (log == 0 || log->disk == 0 || scratch == 0 || log->capacity == 0U ||
            severity > 077U || source > 07777U ||
            payload_words > D6LOG_PAYLOAD_WORDS ||
            (payload_words != 0U && payload == 0))
                return -1;
        blockno = 1U + log->next_slot / D6LOG_RECORDS_BLOCK;
        offset = (log->next_slot % D6LOG_RECORDS_BLOCK) *
            D6LOG_RECORD_WORDS;
        if (d6fs_boot_log_read(log->disk, (kword_t)blockno, scratch) != 0)
                return -1;
        sequence = log->next_sequence;
        scratch[offset + 0U] = D6LOG_RECORD_MAGIC;
        scratch[offset + 1U] = sequence;
        scratch[offset + 2U] = timestamp & D6LOG_WORD_MASK;
        scratch[offset + 3U] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | (kword_t)payload_words;
        for (i = 0U; i < D6LOG_PAYLOAD_WORDS; ++i)
                scratch[offset + 4U + i] = i < payload_words ? payload[i] : 0UL;
        scratch[offset + 7U] = 0UL;
        if (d6fs_boot_log_write(log->disk, (kword_t)blockno, scratch) != 0)
                return -1;
        scratch[offset + 7U] = (D6LOG_RECORD_MAGIC ^ sequence) &
            D6LOG_WORD_MASK;
        if (d6fs_boot_log_write(log->disk, (kword_t)blockno, scratch) != 0)
                return -1;

        if (sequence > (kword_t)log->capacity)
                ++log->lost_records;
        ++log->next_slot;
        if (log->next_slot >= log->capacity)
                log->next_slot = 0U;
        log->next_sequence = (sequence + 1UL) & D6LOG_WORD_MASK;
        if (log->next_sequence == 0UL)
                log->next_sequence = 1UL;
        return d6log_write_header(log, scratch);
}
