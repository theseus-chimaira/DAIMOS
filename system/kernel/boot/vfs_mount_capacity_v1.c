/* KINIT-only mount sizing: no physical I/O, no permanent kernel state. */
#include "vfs_mount_capacity_v1.h"

static unsigned int
vfs_mount_popcount_v1(unsigned int mask)
{
        unsigned int count;

        count = 0U;
        while (mask != 0U) {
                mask &= mask - 1U;
                ++count;
        }
        return count;
}

unsigned int
vfs_mount_capacity_v1(const struct vfs_mount_inventory_v1 *inv)
{
        unsigned int disks;
        unsigned int drums;
        unsigned int tapes;
        unsigned int count;
        unsigned int roots;

        if (inv == 0 || inv->disk_units > 4U || inv->drum_units > 4U ||
            inv->tape_units > 8U || inv->memfs_enabled > 1U)
                return 0U;
        disks = vfs_mount_popcount_v1(inv->root_disk_members);
        drums = vfs_mount_popcount_v1(inv->root_drum_members);
        tapes = vfs_mount_popcount_v1(inv->root_tape_members);
        if (disks > inv->disk_units || drums > inv->drum_units ||
            tapes > inv->tape_units ||
            (inv->root_disk_members >> inv->disk_units) != 0U ||
            (inv->root_drum_members >> inv->drum_units) != 0U ||
            (inv->root_tape_members >> inv->tape_units) != 0U)
                return 0U;
        roots = (disks != 0U) + (drums != 0U) + (tapes != 0U);
        if (roots != 1U)
                return 0U;

        /* Each selected root set occupies one mount. All other units stay
         * potentially mountable, including empty removable transports and
         * drums not proven to be reserved for auxiliary storage. */
        count = inv->disk_units - disks + inv->drum_units - drums +
            inv->tape_units - tapes + inv->memfs_enabled + 1U;
        return count;
}
