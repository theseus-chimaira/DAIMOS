#include "proc_swap.h"
#include "d6fs_provider.h"
#include "dtfs.h"
#include "memfs.h"

int
proc_swap_attach(unsigned int slot, vnode_t backing, kword_t text_words,
    unsigned int pure)
{
        kword_t packed;
        unsigned int provider;
        unsigned int mount;
        unsigned int kind;

        if (proc_swap_records == 0 || slot >= proc_slots ||
            backing == VFS_NODE_NONE)
                return -1;
        provider = VFS_PROVIDER(backing);
        mount = VFS_MOUNT_ID(backing);
        kind = VFS_LOCAL_KIND(backing);
        if (provider < MEMFS_PROVIDER || provider > D6FS_PROVIDER ||
            mount == 0U || mount > VFS_NMOUNT ||
            (provider == MEMFS_PROVIDER && kind != MEMFS_KIND_NODE) ||
            (provider == DTFS_PROVIDER && kind != DTFS_KIND_FILE) ||
            (provider == D6FS_PROVIDER && kind != D6FS_KIND_NODE))
                return -1;
        if (pure == 0U)
                text_words = 0UL;
        if (text_words > PROC_SWAP_TEXT_MASK)
                return -1;
        packed = VFS_INDEX(backing);
        packed |= (kword_t)(mount - 1U) << PROC_SWAP_MOUNT_SHIFT;
        packed |= (kword_t)(provider - MEMFS_PROVIDER) <<
            PROC_SWAP_PROVIDER_SHIFT;
        packed |= text_words << PROC_SWAP_TEXT_SHIFT;
        proc_swap_records[slot].state = packed;
        return 0;
}
