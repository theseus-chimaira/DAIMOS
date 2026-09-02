#include "logstore.h"
#include "diskset_boot.h"

static void
logstore_zero_block(kword_t block[DISKSET_BLOCK_WORDS])
{
        unsigned int i;

        for (i = 0U; i < DISKSET_BLOCK_WORDS; ++i)
                block[i] = 0UL;
}

static kword_t
logstore_record_commit(kword_t sequence, unsigned int payload_words)
{
        kword_t commit;

        commit = (LOGSTORE_RECORD_MAGIC + sequence +
            ((kword_t)payload_words << 18)) & LOGSTORE_WORD_MASK;
        return (commit ^ LOGSTORE_WORD_MASK) | 1UL;
}

static kword_t
logstore_state_commit(kword_t generation, kword_t next_drain_sequence,
    kword_t lost_records, unsigned int capacity)
{
        kword_t commit;

        commit = (LOGSTORE_STATE_MAGIC + generation + next_drain_sequence +
            lost_records + (kword_t)capacity) & LOGSTORE_WORD_MASK;
        return (commit ^ LOGSTORE_WORD_MASK) | 1UL;
}

int
logstore_record_valid(const kword_t block[DISKSET_BLOCK_WORDS])
{
        unsigned int payload_words;

        if (block[0] != LOGSTORE_RECORD_MAGIC || block[1] == 0UL)
                return 0;
        payload_words = (unsigned int)(block[3] & 0777777UL);
        return payload_words <= LOGSTORE_PAYLOAD_WORDS &&
            block[DISKSET_BLOCK_WORDS - 1U] ==
            logstore_record_commit(block[1], payload_words);
}

static int
logstore_state_valid(const kword_t block[DISKSET_BLOCK_WORDS],
    unsigned int capacity)
{
        return block[0] == LOGSTORE_STATE_MAGIC && block[1] != 0UL &&
            block[2] != 0UL && block[4] == (kword_t)capacity &&
            block[DISKSET_BLOCK_WORDS - 1U] ==
            logstore_state_commit(block[1], block[2], block[3], capacity);
}

int
logstore_write_state(struct logstore *log, kword_t next_drain_sequence,
    kword_t lost_records, kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t generation;
        unsigned int copy;

        if (log->state_generation == LOGSTORE_WORD_MASK ||
            next_drain_sequence == 0UL)
                return -1;
        generation = log->state_generation + 1UL;
        copy = log->state_copy <= 1U ? log->state_copy ^ 1U : 0U;
        logstore_zero_block(scratch);
        scratch[0] = LOGSTORE_STATE_MAGIC;
        scratch[1] = generation;
        scratch[2] = next_drain_sequence;
        scratch[3] = lost_records;
        scratch[4] = (kword_t)log->capacity;
        scratch[DISKSET_BLOCK_WORDS - 1U] =
            logstore_state_commit(generation, next_drain_sequence,
            lost_records, log->capacity);
        if (diskset_boot_log_write((kword_t)copy, scratch) != 0)
                return -1;
        log->state_generation = generation;
        log->state_copy = copy;
        log->next_drain_sequence = next_drain_sequence;
        log->lost_records = lost_records;
        return 0;
}

int
logstore_recover(struct logstore *log,
    kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t blocks;
        kword_t best_sequence;
        kword_t oldest_sequence;
        kword_t state_generation;
        kword_t state_drain;
        kword_t state_lost;
        unsigned int capacity;
        unsigned int best_slot;
        unsigned int state_copy;
        unsigned int slot;
        int have_state;

        if (log == 0 || scratch == 0)
                return -1;
        blocks = diskset_boot_log_blocks();
        if (blocks < 3UL)
                return -1;
        capacity = (unsigned int)(blocks - 2UL);

        have_state = 0;
        state_generation = 0UL;
        state_drain = 1UL;
        state_lost = 0UL;
        state_copy = LOGSTORE_STATE_NONE;
        for (slot = 0U; slot < 2U; ++slot) {
                if (diskset_boot_log_read((kword_t)slot, scratch) != 0)
                        return -1;
                if (logstore_state_valid(scratch, capacity) &&
                    (!have_state || scratch[1] > state_generation)) {
                        have_state = 1;
                        state_generation = scratch[1];
                        state_drain = scratch[2];
                        state_lost = scratch[3];
                        state_copy = slot;
                }
        }

        best_sequence = 0UL;
        best_slot = 0U;
        for (slot = 0U; slot < capacity; ++slot) {
                if (diskset_boot_log_read((kword_t)(slot + 2U), scratch) != 0)
                        return -1;
                if (logstore_record_valid(scratch) &&
                    scratch[1] > best_sequence) {
                        best_sequence = scratch[1];
                        best_slot = slot;
                }
        }
        if (best_sequence == LOGSTORE_WORD_MASK)
                return -1;

        log->capacity = capacity;
        log->next_sequence = best_sequence == 0UL ? 1UL : best_sequence + 1UL;
        log->next_slot = best_sequence == 0UL ? 0U : best_slot + 1U;
        if (log->next_slot >= capacity)
                log->next_slot = 0U;
        oldest_sequence = best_sequence >= (kword_t)capacity ?
            best_sequence - (kword_t)capacity + 1UL : 1UL;

        log->state_generation = state_generation;
        log->state_copy = state_copy;
        log->lost_records = state_lost;
        log->next_drain_sequence = state_drain;
        if (!have_state || log->next_drain_sequence < oldest_sequence ||
            log->next_drain_sequence > log->next_sequence)
                log->next_drain_sequence = oldest_sequence;
        return 0;
}

int
logstore_append(struct logstore *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t sequence;
        kword_t overwritten_sequence;
        kword_t lost_delta;
        unsigned int i;

        if (log == 0 || scratch == 0 || log->capacity == 0U ||
            log->next_sequence == 0UL ||
            log->next_sequence == LOGSTORE_WORD_MASK ||
            severity > 077U || source > 07777U ||
            payload_words > LOGSTORE_PAYLOAD_WORDS ||
            (payload_words != 0U && payload == 0))
                return -1;

        sequence = log->next_sequence;
        if (sequence > (kword_t)log->capacity) {
                overwritten_sequence = sequence - (kword_t)log->capacity;
                if (log->next_drain_sequence <= overwritten_sequence) {
                        lost_delta = overwritten_sequence -
                            log->next_drain_sequence + 1UL;
                        if (lost_delta > LOGSTORE_WORD_MASK - log->lost_records ||
                            logstore_write_state(log,
                            overwritten_sequence + 1UL,
                            log->lost_records + lost_delta, scratch) != 0)
                                return -1;
                }
        }

        logstore_zero_block(scratch);
        scratch[0] = LOGSTORE_RECORD_MAGIC;
        scratch[1] = sequence;
        scratch[2] = timestamp & LOGSTORE_WORD_MASK;
        scratch[3] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | (kword_t)payload_words;
        for (i = 0U; i < payload_words; ++i)
                scratch[4U + i] = payload[i] & LOGSTORE_WORD_MASK;
        scratch[DISKSET_BLOCK_WORDS - 1U] =
            logstore_record_commit(sequence, payload_words);
        if (diskset_boot_log_write((kword_t)(log->next_slot + 2U),
            scratch) != 0)
                return -1;

        ++log->next_slot;
        if (log->next_slot >= log->capacity)
                log->next_slot = 0U;
        log->next_sequence = sequence + 1UL;
        return 0;
}
