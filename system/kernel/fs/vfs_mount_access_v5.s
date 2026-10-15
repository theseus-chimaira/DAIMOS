; Staged native mount-table access primitives. Not linked into KCORE.
;
; A contiguous VFS slab has an indexed target table followed by an equally
; sized root table.  Absolute PDP-6 word addresses are supplied by KINIT.
; No dynamic allocation and no implicit global pointer are used here.
;
; vfs_mount_lookup_v5:
;   AC1 = absolute target-table base
;   AC2 = absolute root-table base
;   AC3 = selected mount capacity (1..16)
;   AC4 = target vnode to find (0 is the namespace root target)
; Returns AC1 = matching mounted root, or zero if none. AC5..AC7 scratch.
;
; Inactive until all native callers share the same KINIT-published slab.

        .text
        .globl vfs_mount_lookup_v5
vfs_mount_lookup_v5:
        jumple 3,vfs_mount_lookup_v5_none
        caile 3,020                  ; 16 decimal, 020 octal
        jrst vfs_mount_lookup_v5_none
        setz 5,                      ; zero-based slot
vfs_mount_lookup_v5_loop:
        move 6,1
        add 6,5
        move 7,(6)                  ; candidate target
        camn 7,4
        jrst vfs_mount_lookup_v5_match
vfs_mount_lookup_v5_next:
        aoj 5,
        camge 5,3
        jrst vfs_mount_lookup_v5_loop
vfs_mount_lookup_v5_none:
        setz 1,
        popj 17,
vfs_mount_lookup_v5_match:
        move 6,2
        add 6,5
        move 1,(6)
        jumpn 1,vfs_mount_lookup_v5_done
        jrst vfs_mount_lookup_v5_next
vfs_mount_lookup_v5_done:
        popj 17,
