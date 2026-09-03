#include "logstore.h"
#include "diskset_boot.h"

static kword_t
logstore_state_commit(kword_t generation, kword_t next_sequence,
    kword_t lost_records, unsigned int capacity)
{
        kword_t commit;

        commit = (LOGSTORE_STATE_MAGIC + generation + next_sequence +
            lost_records + (kword_t)capacity) & LOGSTORE_WORD_MASK;
        return (commit ^ LOGSTORE_WORD_MASK) | 1UL;
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

static void
logstore_zero_block(kword_t block[DISKSET_BLOCK_WORDS])
{
        unsigned int i;

        for (i = 0U; i < DISKSET_BLOCK_WORDS; ++i)
                block[i] = 0UL;
}

static kword_t
logstore_oldest_sequence(const struct logstore *log)
{
        kword_t newest;

        if (log->next_sequence <= 1UL)
                return 1UL;
        newest = log->next_sequence - 1UL;
        if (newest >= (kword_t)log->capacity)
                return newest - (kword_t)log->capacity + 1UL;
        return 1UL;
}

static int
logstore_drain_write_state(const struct logstore *log,
    struct logstore_drain *drain, kword_t next_sequence,
    kword_t lost_records, kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t generation;
        unsigned int copy;

        if (drain->state_generation == LOGSTORE_WORD_MASK ||
            next_sequence == 0UL)
                return -1;
        generation = drain->state_generation + 1UL;
        copy = drain->state_copy <= 1U ? drain->state_copy ^ 1U : 0U;
        logstore_zero_block(scratch);
        scratch[0] = LOGSTORE_STATE_MAGIC;
        scratch[1] = generation;
        scratch[2] = next_sequence;
        scratch[3] = lost_records;
        scratch[4] = (kword_t)log->capacity;
        scratch[DISKSET_BLOCK_WORDS - 1U] =
            logstore_state_commit(generation, next_sequence,
            lost_records, log->capacity);
        if (diskset_boot_log_write((kword_t)copy, scratch) != 0)
                return -1;
        drain->state_generation = generation;
        drain->state_copy = copy;
        drain->next_sequence = next_sequence;
        drain->lost_records = lost_records;
        return 0;
}

int
logstore_drain_recover(const struct logstore *log,
    struct logstore_drain *drain, kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t best_generation;
        kword_t oldest;
        unsigned int slot;
        int have_state;

        if (log == 0 || drain == 0 || scratch == 0 || log->capacity == 0U)
                return -1;
        have_state = 0;
        best_generation = 0UL;
        drain->next_sequence = 1UL;
        drain->lost_records = 0UL;
        drain->state_generation = 0UL;
        drain->state_copy = LOGSTORE_STATE_NONE;
        for (slot = 0U; slot < 2U; ++slot) {
                if (diskset_boot_log_read((kword_t)slot, scratch) != 0)
                        return -1;
                if (logstore_state_valid(scratch, log->capacity) &&
                    (!have_state || scratch[1] > best_generation)) {
                        have_state = 1;
                        best_generation = scratch[1];
                        drain->state_generation = scratch[1];
                        drain->next_sequence = scratch[2];
                        drain->lost_records = scratch[3];
                        drain->state_copy = slot;
                }
        }
        oldest = logstore_oldest_sequence(log);
        if (!have_state || drain->next_sequence > log->next_sequence)
                drain->next_sequence = oldest;
        return 0;
}

int
logstore_drain_one(const struct logstore *log,
    struct logstore_drain *drain, logstore_sink_fn sink, void *context,
    kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t oldest;
        kword_t lost;
        kword_t distance;
        unsigned int slot;
        int status;

        if (log == 0 || drain == 0 || sink == 0 || scratch == 0)
                return -1;
        oldest = logstore_oldest_sequence(log);
        if (drain->next_sequence < oldest) {
                lost = oldest - drain->next_sequence;
                if (lost > LOGSTORE_WORD_MASK - drain->lost_records)
                        return -1;
                if (logstore_drain_write_state(log, drain, oldest,
                    drain->lost_records + lost, scratch) != 0)
                        return -1;
        }
        if (drain->next_sequence == log->next_sequence)
                return LOGSTORE_DRAIN_EMPTY;
        if (drain->next_sequence > log->next_sequence)
                return -1;

        distance = log->next_sequence - drain->next_sequence;
        slot = log->next_slot + log->capacity - (unsigned int)distance;
        if (slot >= log->capacity)
                slot -= log->capacity;
        if (diskset_boot_log_read((kword_t)(slot + 2U), scratch) != 0 ||
            !logstore_record_valid(scratch) ||
            scratch[1] != drain->next_sequence)
                return -1;
        status = sink(context, scratch, DISKSET_BLOCK_WORDS);
        if (status != 0)
                return status;
        return logstore_drain_write_state(log, drain,
            drain->next_sequence + 1UL, drain->lost_records, scratch);
}
