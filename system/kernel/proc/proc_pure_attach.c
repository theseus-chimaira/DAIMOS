/**
 * @file proc_pure_attach.c
 * @brief Executable backing record construction independent of swap I/O.
 *
 * Currently compiled into the existing proc_swap object to preserve its ABI.
 */
#include "proc_swap.h"
#include "vfs.h"
#include "memfs.h"
#include "dtfs.h"
#include "tsfs.h"

/* Published VFS mount-root slab; each public mount ID selects slot ID-1.
 * The resident swap record omits provider identity and PURE reload later
 * reconstructs it from this root, so validate the binding at attach time.
 */
extern vnode_t *vfs_mount_root_base;

/** Pack resident executable backing and PURE-text metadata into one record word. */
int
proc_swap_attach(int slot, vnode_t backing, kword_t text_words,
    unsigned int pure)
{
        kword_t packed;
        unsigned int provider;
        unsigned int mount;
        unsigned int kind;

        provider = VFS_PROVIDER(backing);
        mount = VFS_MOUNT_ID(backing);
        kind = VFS_LOCAL_KIND(backing);
        /* Executable-backing provider IDs are deliberately contiguous.
         * Even providers (MEMFS/D6FS) use local node kind 1; odd providers
         * (DTFS/TSFS) use local file/node kind 2.  Keep the one-word swap
         * record validation compact while covering all four providers. */
        if (provider < MEMFS_PROVIDER || provider > TSFS_PROVIDER ||
            mount == 0U || mount > (unsigned int)vfs_mount_limit ||
            kind != 1U + (provider & 1U))
                return -1;
        if (vfs_mount_root_base == 0 ||
            VFS_PROVIDER(vfs_mount_root_base[mount - 1U]) != provider ||
            VFS_MOUNT_ID(vfs_mount_root_base[mount - 1U]) != mount)
                return -1;
        if (pure == 0U)
                text_words = 0UL;
        if (text_words > PROC_SWAP_TEXT_MASK)
                return -1;
        /* v1563: an executable can finish loading after the DTFS
         * unmount preflight. Publish no new PURE dependency (or staged
         * RUN executable) once its mount begins quiescing. No sleep
         * occurs between this admission check and the swap-record
         * publication below. Existing owners are found by the matching
         * SIDL/resident/swapped scan in proc_swap_mount_busy. */
        if (provider == DTFS_PROVIDER &&
            vfs_dtfs_owner_admit_v1562(backing) != 0)
                return -1;
        packed = VFS_INDEX(backing);
        packed |= (kword_t)((mount - 1U) & PROC_SWAP_MOUNT_MASK)
            << PROC_SWAP_MOUNT_SHIFT;
        packed |= text_words << PROC_SWAP_TEXT_SHIFT;
        /* No extra word: save the fifth zero-based mount-slot bit in the
         * existing stable u-area. EXEC staging/rollback must treat this bit
         * as part of the backing record, not as credential information. */
        if (PROC_HAS_UAREA(&proc_table[slot])) {
                if (((mount - 1U) & 020U) != 0U)
                        PROC_CRED_WORD(&proc_table[slot]) |=
                            PROC_SWAP_MOUNT_EXT_BIT;
                else
                        PROC_CRED_WORD(&proc_table[slot]) &=
                            ~PROC_SWAP_MOUNT_EXT_BIT;
        } else if (PROC_STATE(&proc_table[slot]) != PROC_SIDL)
                return -1;
        /* v1563: even a SIDL with an already attached u-area can be in
         * the middle of EXEC. The owner scan must use one authoritative
         * staging bit in both RUN and EXEC, rather than reading a newly
         * allocated or not-yet-copied credential word. SRUN never uses
         * this temporary RH bit; it is its runnable-queue link. */
        if (PROC_STATE(&proc_table[slot]) == PROC_SIDL) {
                proc_table[slot].sched =
                    (proc_table[slot].sched &
                    ~PROC_SWAP_STAGE_MOUNT_EXT_BIT) |
                    (((mount - 1U) & 020U) != 0U ?
                    PROC_SWAP_STAGE_MOUNT_EXT_BIT : 0UL);
        }
        proc_swap_records[slot].state = packed;
        return 0;
}

