/* KINIT-only calculation of future VFS mount-slot demand. */
#ifndef DAIMON_VFS_MOUNT_CAPACITY_V1_H
#define DAIMON_VFS_MOUNT_CAPACITY_V1_H

/* Unit counts describe safely established controller capacity, not media
 * presently inserted. Member masks identify validated selected-root units. */
struct vfs_mount_inventory_v1 {
        unsigned int disk_units;
        unsigned int drum_units;
        unsigned int tape_units;
        unsigned int root_disk_members;
        unsigned int root_drum_members;
        unsigned int root_tape_members;
        unsigned int memfs_enabled;
};

/* Return slots required, or zero for inconsistent inventory. */
unsigned int vfs_mount_capacity_v1(
    const struct vfs_mount_inventory_v1 *inventory);

#endif
