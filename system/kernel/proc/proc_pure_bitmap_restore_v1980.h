#ifndef PURE_BITMAP_RESTORE_V1980_H
#define PURE_BITMAP_RESTORE_V1980_H
#include "proc_pure_ram_restore_v1391.h"
/* Mode3 FULL escrow: original aligned bitmap base and original target
 * base. Both old kind bits reject legacy RAW deferred recovery. */
#define PROC_PURE_BITMAP_FULL_V1980(source, target) \
        (PROC_PURE_FULL_ESCROW_V1917((source), (target)) | PROC_PURE_KIND_MASK_V1919)
/* Slot-zero, TRANSITION held. Success publishes exact full APR and
 * clears TRANSITION. Clean refusal retains mode3 and original unpinned
 * bitmap, zero escrow, TRANSITION held. Uncertain failure retains escrow
 * and TRANSITION; never run old interleaved restore or RAW recovery. */
int proc_pure_bitmap_restore_v1980(unsigned int slot,
    proc_pure_ram_loader_v1391 loader);
/* Release only a clean inactive mode3 transition on slot zero. No
 * backing I/O, pin retirement or deferred transaction recovery. */
int proc_pure_bitmap_unlock_v1981(unsigned int slot);
/* Terminal-only export. Caller excludes returning executive stack;
 * success retires bitmap mode but not common backing/u-area metadata. */
int proc_pure_term_v1982(int slot, struct proc *p, kword_t backing);
#endif
