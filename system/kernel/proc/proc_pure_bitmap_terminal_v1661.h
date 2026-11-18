/* v1661 test-only compressed PURE terminal owner release. */
#ifndef DAIMOS_PBT1661_H
#define DAIMOS_PBT1661_H

#define PROC_STORAGE_BITMAP_V1661 3U

/* Caller owns the process TRANSITION and has entered final teardown.
 * On success ONLY, exact MM compressed owner is freed and mode3 is
 * atomically changed to RESIDENT(0). Swap record/u-area remain owned
 * by the caller. On refusal keep TRANSITION, mode and MM unchanged.
 */
int proc_pure_bitmap_terminal_v1661(unsigned int slot);

#endif
