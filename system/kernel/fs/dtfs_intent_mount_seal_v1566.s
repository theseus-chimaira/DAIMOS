; v1566: exclusive mounted DTFS physical CLEAN commit after VFS owner scan.
;
; Called ONLY through private DTFS MRES op 027 with the serialized
; fs_provider_reg_call lease held, and the VFS v1562 barrier acquired.
; AC1 = zero-based VFS slot, AC2 = exact originally published root vnode.
; Return 0 only after physical DIRTY->CLEAN, pinned-MM cleanup and
; reauthentication of the same root, original media bits, both saved ID
; halves and RW VFS policy. All failures retain the published mount.
;
; MM allocation failure before raw I/O leaves the unit unpoisoned and
; permits a later retry. Once MM is pinned, the shared v1552 wrapper
; marks WRITE_UNCERTAIN BEFORE raw I/O, and that bit remains sticky on
; any failed/partial CLEAN operation, cleanup failure or owner change.
; Clearing uncertain+armed is done only after successful fresh physical
; readback and no remaining MM scratch owner. QUIESCING stays acquired
; until the VFS has redone final owner checks and retired the mount.
; No additional permanent KCORE/MRES DATA/BSS allocation is required.

        .text
        .globl dtfs_intent_mount_seal_v1566
        .globl dtfs_intent_owned_clean_v1566
        .globl vfs_mount_root_base
        .globl vfs_mount_limit
        .globl vfs_mount_policy
        .globl dtfs_media_base
        .globl vfs_mount_aux_base_v1560
        .globl tsfs_file_shape_base
        .globl tsfs_extent_media_base

dtfs_intent_mount_seal_v1566:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                   ; exact zero-based slot
        move    011,2                   ; original mounted DTFS root
        jumpl   010,dtfs_seal_refuse_v1566
        caml    010,vfs_mount_limit
        jrst    dtfs_seal_refuse_v1566
        skipn   1,vfs_mount_root_base
        jrst    dtfs_seal_refuse_v1566
        add     1,010
        move    3,(1)
        came    3,011
        jrst    dtfs_seal_refuse_v1566
        ldb     4,[POINT 6,3,5]
        caie    4,5                     ; exact DTFS provider
        jrst    dtfs_seal_refuse_v1566
        ldb     4,[POINT 6,3,11]
        movei   5,1(010)
        came    4,5                     ; root belongs to this slot
        jrst    dtfs_seal_refuse_v1566

        skipn   012,dtfs_media_base
        jrst    dtfs_seal_refuse_v1566
        add     012,010
        move    013,(012)
        trnn    013,0100000            ; DTVOL2 only
        jrst    dtfs_seal_refuse_v1566
        trnn    013,0020000            ; VFS barrier MUST remain held
        jrst    dtfs_seal_refuse_v1566
        trnn    013,0040000            ; successful prior DIRTY arm
        jrst    dtfs_seal_refuse_v1566
        trne    013,0600030            ; no IO fault or foreign personality
        jrst    dtfs_seal_refuse_v1566

        ; Never issue CLEAN on a read-only VFS mount, not even if a
        ; corrupt caller artificially presents the ARMED in-RAM bit.
        move    5,010
        movn    5,5
        move    6,vfs_mount_policy
        lsh     6,0(5)
        trne    6,1
        jrst    dtfs_seal_refuse_v1566

        skipn   014,vfs_mount_aux_base_v1560
        jrst    dtfs_seal_refuse_v1566
        add     014,010                 ; exact per-slot retained MM basep
        skipn   (014)
        jrst    dtfs_seal_get_id_v1566
        jrst    dtfs_seal_refuse_v1566
dtfs_seal_get_id_v1566:
        skipn   1,tsfs_file_shape_base
        jrst    dtfs_seal_refuse_v1566
        add     1,010
        move    015,(1)
        jumpe   015,dtfs_seal_refuse_v1566
        skipn   1,tsfs_extent_media_base
        jrst    dtfs_seal_refuse_v1566
        add     1,010
        move    016,(1)
        jumpe   016,dtfs_seal_refuse_v1566

        move    1,013
        andi    1,7                     ; exact physical DTC unit
        move    2,015
        move    3,016
        move    4,014                   ; &aux[slot], not scratch pointer
        pushj   17,dtfs_intent_owned_clean_v1566
        jumpn   1,dtfs_seal_refuse_v1566

        ; Physical CLEAN and full MM unpin/free succeeded. All I/O may
        ; have slept; re-check every published owner before clearing a
        ; SINGLE sticky in-RAM uncertainty marker from this operation.
        skipn   (014)
        jrst    dtfs_seal_post_root_v1566
        jrst    dtfs_seal_refuse_v1566
dtfs_seal_post_root_v1566:
        move    1,vfs_mount_root_base
        jumpe   1,dtfs_seal_refuse_v1566
        add     1,010
        move    2,(1)
        came    2,011
        jrst    dtfs_seal_refuse_v1566
        move    1,(012)
        move    2,013
        iori    2,0400000              ; wrapper's fail-closed marker
        came    1,2
        jrst    dtfs_seal_refuse_v1566
        move    1,tsfs_file_shape_base
        jumpe   1,dtfs_seal_refuse_v1566
        add     1,010
        move    2,(1)
        came    2,015
        jrst    dtfs_seal_refuse_v1566
        move    1,tsfs_extent_media_base
        jumpe   1,dtfs_seal_refuse_v1566
        add     1,010
        move    2,(1)
        came    2,016
        jrst    dtfs_seal_refuse_v1566
        move    5,010
        movn    5,5
        move    6,vfs_mount_policy
        lsh     6,0(5)
        trne    6,1
        jrst    dtfs_seal_refuse_v1566

        ; ONLY successful physical CLEAN + valid owner may clear these
        ; two RAM bits; retain v1562 QUIESCING until VFS retires safely.
        movei   2,0440000             ; WR_UNCERT | INTENT_ARMED
        andcam  2,(012)
        setz    1,
        jrst    dtfs_seal_done_v1566
dtfs_seal_refuse_v1566:
        seto    1,
dtfs_seal_done_v1566:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
