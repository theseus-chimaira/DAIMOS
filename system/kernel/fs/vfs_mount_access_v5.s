; Staged native mount-table access primitives. Not linked into KCORE.
;
; A contiguous VFS slab has an indexed target table followed by an equally
; sized root table.  Absolute PDP-6 word addresses are supplied by KINIT.
; No dynamic allocation and no implicit global pointer are used here.
;
; vfs_mount_lookup_v5:
;   AC1 = absolute target-table base
;   AC2 = absolute root-table base
;   AC3 = selected mount capacity (1..26)
;   AC4 = target vnode to find (0 is the namespace root target)
; Returns AC1 = matching mounted root, or zero if none. AC5..AC7 scratch.
;
; Unlinked prototype: production native VFS already uses MM-backed bases.

        .text
        .globl vfs_mount_lookup_v5
        .globl vfs_mount_slot_read_v5
        .globl vfs_mount_slot_write_v5
        .globl vfs_mount_slot_clear_v5

; Common indexed access ABI:
;   AC1 = table base, AC2 = capacity (1..26), AC3 = slot (0..capacity-1)
; READ:  AC4 <- word, AC1 <- 0 on success or -1 on invalid access.
; WRITE: AC4 = word; AC1 <- 0 on success, -1 on invalid access.
; CLEAR: AC1 <- 0 on success, -1 on invalid access.
; AC5 is scratch. AC4 is unchanged on READ failure. A separate status avoids
; reserving any special sentinel from the full 36-bit value domain.
vfs_mount_slot_read_v5:
        pushj 17,vfs_mount_slot_addr_v5
        jumpl 5,vfs_mount_slot_invalid_v5
        move 4,(5)
        setz 1,
        popj 17,
vfs_mount_slot_write_v5:
        pushj 17,vfs_mount_slot_addr_v5
        jumpl 5,vfs_mount_slot_invalid_v5
        movem 4,(5)
        setz 1,
        popj 17,
vfs_mount_slot_clear_v5:
        pushj 17,vfs_mount_slot_addr_v5
        jumpl 5,vfs_mount_slot_invalid_v5
        setzm (5)
        setz 1,
        popj 17,
vfs_mount_slot_invalid_v5:
        seto 1,
        popj 17,

; Internal address calculation does not touch AC1..AC4.
vfs_mount_slot_addr_v5:
        jumple 2,vfs_mount_slot_addr_bad_v5
        caile 2,032                 ; at most 26 mount slots
        jrst vfs_mount_slot_addr_bad_v5
        jumpl 3,vfs_mount_slot_addr_bad_v5
        caml 3,2
        jrst vfs_mount_slot_addr_bad_v5
        move 5,1
        add 5,3
        popj 17,
vfs_mount_slot_addr_bad_v5:
        seto 5,
        popj 17,

vfs_mount_lookup_v5:
        jumple 3,vfs_mount_lookup_v5_none
        caile 3,032                  ; 26 decimal, 032 octal
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
