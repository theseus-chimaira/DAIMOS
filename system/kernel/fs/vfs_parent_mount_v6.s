; Staged native replacement for VFS parent mount-boundary resolution.
; This contains the mount-specific logic of vfs_parent/vfs_parent_name,
; but is not linked until the common slab addresses are published by KINIT.
;
; AC1 = vnode; AC2 = absolute root table; AC3 = absolute target table;
; AC4 = selected capacity, 1..16.
; Returns AC1 = containing mount target when the vnode is exactly a mounted
; root; otherwise AC1 is unchanged. AC5..AC7 scratch.
;
; The vnode mount-id field occupies bits 24..29 (PDP-10 bit numbering).
; The slot is the public mount ID minus one. Zero is built-in namespace.

        .text
        .globl vfs_parent_mount_v6
vfs_parent_mount_v6:
        ldb 5,[POINT 6,1,11]
        sojl 5,vfs_parent_mount_v6_done
        jumple 4,vfs_parent_mount_v6_done
        caile 4,020
        jrst vfs_parent_mount_v6_done
        caml 5,4
        jrst vfs_parent_mount_v6_done
        move 6,2
        add 6,5
        move 7,(6)
        came 7,1
        jrst vfs_parent_mount_v6_done
        move 6,3
        add 6,5
        move 1,(6)
vfs_parent_mount_v6_done:
        popj 17,
