#include "d6fs_boot.h"
#include "d6fs_provider.h"
#include "kinit.h"
#include "blockset_boot.h"
#include "auxstore.h"
#include "blockset_layout.h"
#include "dsk270.h"
#include "fs_mres.h"
#include "logstore.h"
#include "fs_backing.h"
#if KINIT_FULL
#include "drm236.h"
#include "root_select.h"
#endif

#define ROOT_D6FS_UNITS      4U
#define ROOT_SCAN_LIMIT      0200U
#define ROOT_INFO_VALID      0400000000000UL
#define ROOT_INFO_LOC_SHIFT  9U
#define ROOT_INFO_MASK_SHIFT 2U

/* KINIT-only scratch used while probing and mounting the root D6FS.
 * It is reclaimed with KINIT and does not consume resident FS storage. */
static kword_t d6fs_boot_scratch[D6FS_BLOCK_WORDS];
static kword_t root_info[ROOT_D6FS_UNITS];

/**
 * @brief Scan one physical D6FS-capable unit for a root blockset member.
 *
 * Disk units may place the D6FS root marker within the first ROOT_SCAN_LIMIT
 * sectors.  Drum roots use block zero, so the DRM path performs a single
 * probe.  A valid member must identify a logical index represented in its own
 * membership mask.
 *
 * On success root_info[unit] receives the packed member description.  Failure
 * leaves that entry zero.
 *
 * @param root_class Physical controller class, DSK or DRM.
 * @param unit Physical unit number to inspect.
 */
static void
d6fs_boot_scan_unit(unsigned int root_class, unsigned int unit)
{
        kword_t *block;
        kword_t member;
        unsigned int index;
        unsigned int mask;
        unsigned int sector;

#if !KINIT_FULL
        (void)root_class;
#endif
        block = d6fs_boot_block_buffer();
        root_info[unit] = 0UL;
#if KINIT_FULL
        for (sector = 0U; sector < (root_class == KINIT_ROOT_DRM ? 1U :
            ROOT_SCAN_LIMIT); ++sector) {
                if ((root_class == KINIT_ROOT_DRM ?
                    drm236_read_block(unit, (kword_t)sector, block) :
                    dsk270_read_sector(unit, (kword_t)sector, block)) != 0)
                        return;
#else
        for (sector = 0U; sector < ROOT_SCAN_LIMIT; ++sector) {
                if (dsk270_read_sector(unit, (kword_t)sector, block) != 0)
                        return;
#endif
                if (block[BLOCKSET_LAYOUT_MAGIC_WORD] !=
                    BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                        continue;
                member = block[5];
                mask = (unsigned int)((member >> 20U) & 017U);
                index = (unsigned int)((member >> 16U) & 017U);
                if (index >= ROOT_D6FS_UNITS ||
                    (mask & (1U << index)) == 0U)
                        continue;
                root_info[unit] = ROOT_INFO_VALID |
                    ((kword_t)sector << ROOT_INFO_LOC_SHIFT) |
                    ((kword_t)mask << ROOT_INFO_MASK_SHIFT) |
                    (kword_t)index;
                return;
        }
}

/**
 * @brief Discover and select one complete D6FS root set.
 *
 * Units are scanned incrementally in physical order.  A set becomes a
 * candidate only at its lowest logical member and only after all members named
 * by its membership mask have been found.  The selected physical layout is
 * encoded into the two-word KINIT boot handoff for blockset_boot_discover().
 *
 * @param root_class KINIT_ROOT_DSK or KINIT_ROOT_DRM.
 * @param ordinal Zero-based complete-set ordinal within that controller class.
 * @return 0 when a complete set was selected, -1 when none matched.
 */
int
d6fs_boot_select(unsigned int root_class, unsigned int ordinal)
{
        kword_t handoff[2];
        kword_t info;
        unsigned int index;
        unsigned int mask;
        unsigned int present;
        unsigned int found;
        unsigned int unit;
        unsigned int physical;

        for (unit = 0U; unit < ROOT_D6FS_UNITS; ++unit) {
                d6fs_boot_scan_unit(root_class, unit);
                found = 0U;
                for (physical = 0U; physical <= unit; ++physical) {
                        info = root_info[physical];
                        if ((info & ROOT_INFO_VALID) == 0UL)
                                continue;
                        mask = (unsigned int)((info >> ROOT_INFO_MASK_SHIFT) &
                            017U);
                        index = (unsigned int)(info & 03U);
                        if ((mask & ((1U << index) - 1U)) != 0U)
                                continue;
                        handoff[0] = 0777777777777UL;
                        handoff[1] = 0777777777777UL;
                        present = 0U;
                        for (index = 0U; index <= unit; ++index) {
                                kword_t candidate;
                                kword_t half;
                                unsigned int cindex;

                                candidate = root_info[index];
                                if ((candidate & ROOT_INFO_VALID) == 0UL ||
                                    (unsigned int)((candidate >>
                                    ROOT_INFO_MASK_SHIFT) & 017U) != mask)
                                        continue;
                                cindex = (unsigned int)(candidate & 03U);
                                present |= 1U << cindex;
                                half = ((kword_t)index << 16U) |
                                    ((candidate >> ROOT_INFO_LOC_SHIFT) &
                                    0177777UL);
                                if ((cindex & 1U) == 0U)
                                        handoff[cindex / 2U] =
                                            (handoff[cindex / 2U] & 0777777UL) |
                                            (half << 18U);
                                else
                                        handoff[cindex / 2U] =
                                            (handoff[cindex / 2U] &
                                            0777777000000UL) | half;
                        }
                        if (present != mask)
                                continue;
                        if (found++ != ordinal)
                                continue;
                        kinit_boot_handoff[0] = handoff[0];
                        kinit_boot_handoff[1] = handoff[1];
                        return 0;
                }
        }
        return -1;
}

kword_t *
d6fs_boot_block_buffer(void)
{
        return d6fs_boot_scratch;
}

static int
d6fs_boot_runtime_init(const struct d6fs_super_info *super,
    kword_t alloc_cursor, kword_t summary_start, unsigned int flags,
    kword_t super_a, kword_t super_b, unsigned int copy)
{
        struct d6fs_reader *reader;
        vnode_t root;
        unsigned int id;
        unsigned int writable;
        kword_t *scratch;
        kword_t dirty_block;

        if (super == 0 || copy > 1U || super_a == super_b ||
            super->total_blocks == 0UL || super->fcb_count == 0UL ||
            super->root_fcb >= super->fcb_count ||
            d6fs_active_reader == 0 ||
            d6fs_backing_read_addr == 0U)
                return -1;

        writable = (flags & VFS_MOUNT_RDONLY) == 0U &&
            d6fs_backing_write_addr != 0U && blockset_boot_writable() > 0;
        if (!writable)
                flags |= VFS_MOUNT_RDONLY;

        /* Disk boot owns initial namespace creation.  There is no
         * temporary root to replace or rebind. */
        if (vfs_namespace_root != VFS_NODE_NONE)
                return -1;
        if (vfs_mount(VFS_NODE_NONE, D6FS_PROVIDER, D6FS_KIND_NODE,
            super->root_fcb, flags, &root) != 0)
                return -1;
        id = VFS_MOUNT_ID(root);
        reader = d6fs_active_reader;
        if (reader == 0 || id != 1U || reader->opaque != 0) {
                (void)vfs_unmount(root);
                return -1;
        }

        reader->alloc_cursor = alloc_cursor;
        reader->opaque = (summary_start << D6FS_PROVIDER_SUMMARY_SHIFT) |
            (kword_t)id |
            (kword_t)(copy ? D6FS_PROVIDER_MOUNT_COPY : 0U);
        reader->sequence = super->sequence;
        reader->total_blocks = super->total_blocks;
        reader->root_fcb = super->root_fcb;
        reader->fcb_start = super->fcb_start;
        reader->fcb_count = super->fcb_count;
        reader->freemap_start = super->freemap_start;
        reader->freemap_blocks = super->freemap_blocks;
        reader->backing.blocks = super->total_blocks;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 0U) = super_a;
        D6FS_RUNTIME_SUPER_BLOCK(reader, 1U) = super_b;

        if (writable) {
                kword_t selected_block;

                dirty_block = copy == 0U ? super_b : super_a;
                selected_block = copy == 0U ? super_a : super_b;
                scratch = d6fs_boot_block_buffer();
                /* Rebuild the alternate from the selected valid superblock.
                 * The alternate may be corrupt; mutating it in place would
                 * preserve a bad magic/layout and make the new DIRTY
                 * generation invalid. */
                if (blockset_boot_read(selected_block, scratch) != 0)
                        goto fail;
                scratch[D6FS_SB_SEQUENCE] = super->sequence + 1UL;
                scratch[D6FS_SB_STATE] = D6FS_STATE_DIRTY;
                if (blockset_boot_write(dirty_block, scratch) != 0)
                        goto fail;
                reader->sequence = super->sequence + 1UL;
                reader->opaque ^= (kword_t)D6FS_PROVIDER_MOUNT_COPY;
                reader->opaque |= (kword_t)D6FS_PROVIDER_MOUNT_WRITABLE;
        }

        return 0;

fail:
        /* Preserve the mount id until VFS/provider teardown has released the
         * dynamic reader.  PREPARE_UNMOUNT owns reader destruction. */
        (void)vfs_unmount(root);
        return -1;
}

int
d6fs_boot_mount_root(unsigned int flags)
{
        kword_t a[D6FS_SUPER_WORDS];
        kword_t b[D6FS_SUPER_WORDS];
        struct d6fs_super_info super;
        const kword_t *selected;
        kword_t high;
        kword_t swap_blocks;
        kword_t log_start;
        kword_t log_blocks;
        kword_t tail_blocks;
        kword_t *scratch;
        kword_t super_a;
        kword_t super_b;
        kword_t total;
        kword_t alloc_cursor;
        kword_t summary_start;
        unsigned int copy;
        unsigned int i;
        int rc;
        struct vfs_stat st;

        scratch = d6fs_boot_block_buffer();
        rc = blockset_boot_discover(&super_a, &super_b);
        if (rc != 0)
                return rc;
        if (d6fs_active_reader != 0) {
                unsigned int direct_unit;
                kword_t direct_base;
                kword_t direct_blocks;
                kword_t direct_tail;
                struct d6fs_reader *reader;

                reader = d6fs_active_reader;
                if (reader == 0)
                        return -1;
                if (blockset_boot_direct(&direct_unit, &direct_base,
                    &direct_blocks, &direct_tail) != 0) {
                        reader->backing.opaque = ((kword_t)direct_unit << 18U) |
                            direct_base;
                        reader->backing.opaque |=
                            (kword_t)FS_BACKING_DIRECT_ROOT_TAG << 18U;
#if KINIT_FULL
                        if (root_select_class() == KINIT_ROOT_DRM)
                                reader->backing.opaque |=
                                    (kword_t)FS_BACKING_DIRECT_DRM_TAG << 18U;
#endif
                }
        }
        total = blockset_boot_blocks();
        if (total == 0UL || super_a >= total || super_b >= total ||
            super_a == super_b || blockset_boot_read(super_a, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                a[i] = scratch[i];
        if (blockset_boot_read(super_b, scratch) != 0)
                return -1;
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                b[i] = scratch[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0)
                return -1;
        selected = copy == 0U ? a : b;
        summary_start = selected[D6FS_SB_SUMMARY_START];
        alloc_cursor = summary_start + selected[D6FS_SB_SUMMARY_BLOCKS];
        if (alloc_cursor >= super.total_blocks)
                alloc_cursor = 0UL;
        high = selected[D6FS_SB_RESERVATION_HIGH];
        swap_blocks = (((high >> D6FS_RES_SWAP_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (selected[D6FS_SB_SWAP_RESERVATION] &
            D6FS_RESERVATION_LEN_LOW_MASK);
        tail_blocks = blockset_direct_tail;
        if (auxstore_backstore_blocks != 0UL) {
                unsigned int unit;
                unsigned int member;
                kword_t base;
                kword_t blocks;
                kword_t tail;

                tail_blocks = 0UL;
                for (member = 0U; member < BLOCKSET_BOOT_MEMBERS;
                    ++member) {
                        if (!blockset_boot_member(member, &unit, &base,
                            &blocks, &tail))
                                break;
                        tail_blocks += tail;
                }
        }
        if ((swap_blocks == 0UL && tail_blocks != 0UL) ||
            swap_blocks > tail_blocks)
                return -1;
        if (auxstore_backstore_blocks == 0UL) {
                blockset_direct_blocks = total;
                blockset_direct_tail = swap_blocks;
        }
        log_start = selected[D6FS_SB_LOG_RESERVATION] >>
            D6FS_RESERVATION_START_SHIFT;
        log_blocks = (((high >> D6FS_RES_LOG_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (selected[D6FS_SB_LOG_RESERVATION] &
            D6FS_RESERVATION_LEN_LOW_MASK);
        if (log_blocks != 0UL &&
            ((super_a >= log_start && super_a - log_start < log_blocks) ||
            (super_b >= log_start && super_b - log_start < log_blocks)))
                return -1;
        if (auxstore_backstore_blocks == 0UL)
                blockset_direct_tail = swap_blocks;
        if (auxstore_logstore_blocks == 0UL)
                logstore_boot_configure(log_start, log_blocks);
        if (swap_blocks != 0UL && auxstore_backstore_blocks == 0UL)
                flags |= VFS_MOUNT_STORAGE_SWAP;
        if (log_blocks != 0UL && auxstore_logstore_blocks == 0UL)
                flags |= VFS_MOUNT_STORAGE_LOGSTORE;
        rc = d6fs_boot_runtime_init(&super, alloc_cursor, summary_start,
            flags, super_a, super_b, copy);
        if (rc != 0) {
                if (auxstore_backstore_blocks == 0UL)
                        blockset_direct_tail = 0UL;
                if (auxstore_logstore_blocks == 0UL)
                        logstore_boot_configure(0UL, 0UL);
                return rc;
        }
        if (vfs_namespace_root == VFS_NODE_NONE ||
            vfs_stat(vfs_namespace_root, &st) != 0 ||
            st.type != VFS_TYPE_DIR) {
                if (vfs_namespace_root != VFS_NODE_NONE) {
                        i = VFS_MOUNT_ID(vfs_namespace_root);
                        (void)vfs_storage_release(i,
                            flags & VFS_MOUNT_STORAGE_MASK);
                        if (auxstore_backstore_blocks == 0UL)
                                blockset_direct_tail = 0UL;
                        if (auxstore_logstore_blocks == 0UL)
                                logstore_boot_configure(0UL, 0UL);
                        (void)vfs_unmount(vfs_namespace_root);
                }
                return -1;
        }
        return 0;
}
