/**
 * @file proc_pure_admit_v1608.h
 * @brief P1-E admission of an immutable PURE source on a swapless boot.
 *
 * The caller holds the process TRANSITION bit and has validated a stable
 * u-area. The backing record was published by proc_swap_attach(), which
 * also validated the provider and performed DTFS mount admission.
 *
 * This helper repeats the non-sleeping mount-identity check at the point
 * of RAW RAM eviction or reconstruction. It is deliberately independent
 * of the PDP-6 relocation base, PURE prefix alignment and physical MM
 * representation. Existing VFS backing scans protect executable identity
 * after publication; this helper does not replace their lifetime locks.
 *
 * Do not require this admission check for terminal PACKED teardown: a
 * dead or unavailable backing provider must not prevent MM owner release.
 * Source-linked staging only; no additional resident KCORE code or state.
 */
#ifndef DAIMOS_PROC_PURE_ADMIT_V1608_H
#define DAIMOS_PROC_PURE_ADMIT_V1608_H

#include "proc_swap.h"
#include "bstore.h"
#include "vfs.h"
#include "memfs.h"
#include "tsfs.h"
#include "mm.h"

extern vnode_t *vfs_mount_root_base;

static int
pure_admit_source_v1608(const struct proc *p, kword_t backing)
{
        unsigned int mount;
        unsigned int provider;
        kword_t *u;
        vnode_t root;

        /*
         * A disabled but still owned BACKSTORE is not a swapless machine.
         * Do not change the meaning of the existing disk-backed mode.
         * KINIT's no-device path publishes all four fields as zero.
         */
        if (backstore_enabled || backstore_blocks ||
            backstore_blocks_used || backstore_bitmap ||
            proc_swap_blocks_used)
                return MM_ERR_INVAL;
        if (!p || !PROC_HAS_UAREA(p) || !PROC_UAREA_BASE(p) ||
            !backing || !vfs_mount_root_base)
                return MM_ERR_INVAL;

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
        u = (kword_t *)(unsigned long)
            (PROC_UAREA_BASE(p) * sizeof(kword_t));
#else
        u = (kword_t *)(unsigned long)PROC_UAREA_BASE(p);
#endif
        mount = (unsigned int)((backing >> PROC_SWAP_MOUNT_SHIFT) &
            PROC_SWAP_MOUNT_MASK);
        if (u[PROC_CRED_OFFSET] & PROC_SWAP_MOUNT_EXT_BIT)
                mount |= 020U;
        ++mount; /* zero-based backing slot -> public mount ID */
        if (mount > (unsigned int)vfs_mount_limit)
                return MM_ERR_INVAL;
        root = vfs_mount_root_base[mount - 1U];
        provider = VFS_PROVIDER(root);
        if (provider < MEMFS_PROVIDER || provider > TSFS_PROVIDER ||
            VFS_MOUNT_ID(root) != mount)
                return MM_ERR_INVAL;
        return MM_OK;
}

#endif
