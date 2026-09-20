#include "logstore.h"
#include "storage.h"

/* Supplied by the eventual runtime drain MRES when tape support is resident. */
extern int logstore_tape_io(unsigned int unit, int op, kword_t *buffer,
    unsigned int words);

int
logstore_mtc_sink(void *context,
    const kword_t record[BLOCKSET_BLOCK_WORDS], unsigned int words)
{
        unsigned int unit;
        int status;

        if (context == 0 || record == 0 || words != BLOCKSET_BLOCK_WORDS)
                return -1;
        unit = *(const unsigned int *)context;
        status = logstore_tape_io(unit, MTC_OP_STATUS, 0, 0U);
        if ((status & MTC_ST_TAPE_FREE) == 0 ||
            (status & MTC_ST_EOT) != 0)
                return LOGSTORE_DRAIN_REEL;
        return logstore_tape_io(unit, MTC_OP_WRITE, (kword_t *)record, words);
}
