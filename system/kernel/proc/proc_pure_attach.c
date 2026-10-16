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
        if (pure == 0U)
                text_words = 0UL;
        if (text_words > PROC_SWAP_TEXT_MASK)
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
        } else {
                /* RUN loads the executable before allocating the child's
                 * u-area. SIDL's unused RH link bit is its one-bit staging
                 * register until credentials are copied and RUN publishes
                 * the final SRUN descriptor. No added resident word. */
                if (PROC_STATE(&proc_table[slot]) != PROC_SIDL)
                        return -1;
                proc_table[slot].sched =
                    (proc_table[slot].sched &
                    ~PROC_SWAP_STAGE_MOUNT_EXT_BIT) |
                    (((mount - 1U) & 020U) != 0U ?
                    PROC_SWAP_STAGE_MOUNT_EXT_BIT : 0UL);
        }
        proc_swap_records[slot].state = packed;
        return 0;
}

