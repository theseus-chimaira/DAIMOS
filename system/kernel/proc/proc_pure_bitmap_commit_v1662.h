/* v1662 opt-in prepared PURE bitmap commit; NOT runnable mode3. */
#ifndef DAIMOS_PBC1662_H
#define DAIMOS_PBC1662_H
#include "mm.h"

/* Source must be the single exactly ONCE-pinned RAW suffix; target
 * must be the separately prepared, verified-length, UNPINNED bitmap
 * owner. Caller holds TRANSITION on a stopped/sleeping process.
 * On success source MM owner is retired and mode2 -> mode3 is
 * published under a single PI exclusion, without clearing TRANSITION.
 * On refusal neither owner, pin nor process mode is changed.
 */
int proc_pure_bitmap_commit_v1662(unsigned int slot, kword_t raw_base,
    kword_t encoded_base, kword_t encoded_words);
#endif
