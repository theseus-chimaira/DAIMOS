#include "logstore.h"
#include "diskset_boot.h"
#include "storage.h"

/*
 * The eventual runtime LOGSTORE MRES supplies this one narrow bridge to the
 * relocated STORAGE service.  Keeping it outside KINIT avoids permanent glue
 * and lets the caller lend the sole 128-word transfer buffer.
 */
extern int logstore_tape_io(unsigned int unit, int op, kword_t *buffer,
    unsigned int words);

int
logstore_drain_one(struct logstore *log, unsigned int unit,
    kword_t scratch[DISKSET_BLOCK_WORDS])
{
        kword_t distance;
        int slot;
        int status;

        if (log->next_drain_sequence == log->next_sequence)
                return LOGSTORE_DRAIN_EMPTY;

        /* Recovery and append preserve distance <= capacity. */
        distance = log->next_sequence - log->next_drain_sequence;
        slot = (int)(log->next_slot + log->capacity -
            (unsigned int)distance);
        if (slot >= (int)log->capacity)
                slot -= (int)log->capacity;
        if (diskset_boot_log_read((kword_t)(slot + 2U), scratch) != 0 ||
            !logstore_record_valid(scratch) ||
            scratch[1] != log->next_drain_sequence)
                return -1;

        status = logstore_tape_io(unit, MTC_OP_STATUS, 0, 0U);
        if ((status & MTC_ST_TAPE_FREE) == 0 ||
            (status & MTC_ST_EOT) != 0)
                return LOGSTORE_DRAIN_REEL;
        status = logstore_tape_io(unit, MTC_OP_WRITE, scratch,
            DISKSET_BLOCK_WORDS);
        if (status != 0)
                return status;
        return logstore_write_state(log, log->next_drain_sequence + 1UL,
            log->lost_records, scratch);
}
