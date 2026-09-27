#if !defined(KINIT_FULL) || KINIT_FULL
#include "logstore.h"

static void
logstore_zero_block(kword_t block[BLOCKSET_BLOCK_WORDS])
{
        unsigned int i;

        for (i = 0U; i < BLOCKSET_BLOCK_WORDS; ++i)
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

int
logstore_record_valid(const kword_t block[BLOCKSET_BLOCK_WORDS])
{
        unsigned int payload_words;

        if (block[0] != LOGSTORE_RECORD_MAGIC || block[1] == 0UL)
                return 0;
        payload_words = (unsigned int)(block[3] & 0777777UL);
        return payload_words <= LOGSTORE_PAYLOAD_WORDS &&
            block[BLOCKSET_BLOCK_WORDS - 1U] ==
            logstore_record_commit(block[1], payload_words);
}

int
logstore_recover(struct logstore *log,
    kword_t scratch[BLOCKSET_BLOCK_WORDS])
{
        kword_t blocks;
        kword_t best_sequence;
        unsigned int capacity;
        unsigned int best_slot;
        unsigned int slot;

        if (log == 0 || scratch == 0)
                return -1;
        blocks = logstore_boot_blocks();
        if (blocks < 3UL)
                return -1;
        capacity = (unsigned int)(blocks - 2UL);

        best_sequence = 0UL;
        best_slot = 0U;
        for (slot = 0U; slot < capacity; ++slot) {
                if (logstore_boot_read((kword_t)(slot + 2U), scratch) != 0)
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
        return 0;
}

int
logstore_append(struct logstore *log, unsigned int severity,
    unsigned int source, kword_t timestamp, const kword_t *payload,
    unsigned int payload_words, kword_t scratch[BLOCKSET_BLOCK_WORDS])
{
        kword_t sequence;
        unsigned int i;

        if (log == 0 || scratch == 0 || log->capacity == 0U ||
            log->next_sequence == 0UL ||
            log->next_sequence == LOGSTORE_WORD_MASK ||
            severity > 077U || source > 07777U ||
            payload_words > LOGSTORE_PAYLOAD_WORDS ||
            (payload_words != 0U && payload == 0))
                return -1;

        sequence = log->next_sequence;
        logstore_zero_block(scratch);
        scratch[0] = LOGSTORE_RECORD_MAGIC;
        scratch[1] = sequence;
        scratch[2] = timestamp & LOGSTORE_WORD_MASK;
        scratch[3] = ((kword_t)severity << 30) |
            ((kword_t)source << 18) | (kword_t)payload_words;
        for (i = 0U; i < payload_words; ++i)
                scratch[4U + i] = payload[i] & LOGSTORE_WORD_MASK;
        scratch[BLOCKSET_BLOCK_WORDS - 1U] =
            logstore_record_commit(sequence, payload_words);
        if (logstore_boot_write((kword_t)(log->next_slot + 2U),
            scratch) != 0)
                return -1;

        ++log->next_slot;
        if (log->next_slot >= log->capacity)
                log->next_slot = 0U;
        log->next_sequence = sequence + 1UL;
        return 0;
}

#else
typedef int logstore_lowmem_no_code_t;
#endif
