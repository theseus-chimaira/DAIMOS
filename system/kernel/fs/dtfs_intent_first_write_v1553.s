; v1553: authenticated DTFS per-mount first-write intent state transition.
;
; This code is deliberately NOT linked into the production DTFS provider.
; It cannot make DTVOL2 writable until mount admission and exclusive CLEAN
; unmount are also in place. All data/dir paths must eventually call this
; through the *same* serialized fs_provider_reg_call lease.
;
; AC1 = zero-based published VFS DTFS mount slot (NOT raw DTC unit).
; AC2 = pointer to the provider's retained one-word MM scratch base.
; Caller holds the exclusive provider lease across the entire call.
; Returns AC1=0 only when the physical DTVOL2 DIRTY marker has been
; verified (fresh CLEAN->DIRTY arm, or already-DIRTY raw readback)
; and the temporary MM ownership has been released successfully.
; Returns AC1=-1 otherwise. No state changes on a failed arm; the v1552
; helper preserves any MM extent whose cleanup was refused in *AC2.
;
; The physically verified marker survives process crash, discard and boot.
; The ARMED bit is only an in-RAM transaction state: every subsequent
; write revalidates the physical RBN0 and original identity (v1554).
; This remains UNLINKED; actual media-change races between the final
; RBN0 read and the caller's DATA/DIR write need integration acceptance.

        .text
        .globl dtfs_intent_first_write_v1553
        .globl dtfs_intent_owned_arm_v1552
        .globl dtfs_intent_owned_verify_v1554
        .globl vfs_mount_limit
        .globl vfs_mount_root_base
        .globl dtfs_media_base
        .globl tsfs_file_shape_base
        .globl tsfs_extent_media_base

        .equ DTFS_V2_FORMAT_FLAG_V1553,0100000
        .equ DTFS_V2_ARMED_FLAG_V1553,0040000
        .equ DTFS_V2_IO_SUSPECT_FLAGS_V1553,0600000

dtfs_intent_first_write_v1553:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                   ; exact zero-based mount slot
        move    011,2                   ; retained scratch MM basep
        jumpl   010,dtfs_first_refuse_v1553
        caml    010,vfs_mount_limit
        jrst    dtfs_first_refuse_v1553
        jumpe   011,dtfs_first_refuse_v1553

        ; Authenticate the still-published vnode provider and mount ID.
        ; A slot with only a leftover media word is NOT a valid mount.
        skipn   1,vfs_mount_root_base
        jrst    dtfs_first_refuse_v1553
        add     1,010
        skipn   3,(1)
        jrst    dtfs_first_refuse_v1553
        ldb     4,[POINT 6,3,5]
        caie    4,5                     ; DTFS provider, not TSFS/D6FS
        jrst    dtfs_first_refuse_v1553
        ldb     4,[POINT 6,3,11]
        movei   5,1(010)
        came    4,5                     ; exact mounted VFS slot
        jrst    dtfs_first_refuse_v1553

        skipn   012,dtfs_media_base
        jrst    dtfs_first_refuse_v1553
        add     012,010
        move    013,(012)
        trne    013,030                 ; native only, no TENEX/ITS
        jrst    dtfs_first_refuse_v1553
        trnn    013,DTFS_V2_FORMAT_FLAG_V1553
        jrst    dtfs_first_refuse_v1553
        trne    013,DTFS_V2_IO_SUSPECT_FLAGS_V1553
        jrst    dtfs_first_refuse_v1553
        move    014,013
        andi    014,7                   ; actual physical DTC unit

        ; The saved 72-bit ID is the physically admitted mount identity.
        ; Zero halves are legacy/invalid and are never writable in V2.
        skipn   1,tsfs_file_shape_base
        jrst    dtfs_first_refuse_v1553
        add     1,010
        move    015,(1)
        jumpe   015,dtfs_first_refuse_v1553
        skipn   1,tsfs_extent_media_base
        jrst    dtfs_first_refuse_v1553
        add     1,010
        move    016,(1)
        jumpe   016,dtfs_first_refuse_v1553

        ; Never overwrite a caller-retained leaked MM extent. A failed
        ; unpin/free from v1552 must remain visible to the provider.
        skipn   (011)
        jrst    dtfs_first_check_armed_v1553
        jrst    dtfs_first_refuse_v1553
dtfs_first_check_armed_v1553:
        trne    013,DTFS_V2_ARMED_FLAG_V1553
        jrst    dtfs_first_verify_armed_v1554

        ; First write: actual v1552 MM alloc/pin + v1551 physical mark.
        ; Only confirmed on-media DIRTY plus complete scratch release
        ; may set the per-mount ARMED bit. A failed attempt never does.
        move    1,014
        move    2,015
        move    3,016
        move    4,011
        pushj   17,dtfs_intent_owned_arm_v1552
        jumpn   1,dtfs_first_refuse_v1553

        ; The serialized provider lease excludes a second DTFS caller.
        ; Still require exactly the same authenticated media word.
        pushj   17,dtfs_first_post_io_check_v1554
        jumpn   1,dtfs_first_refuse_v1553
        movei   1,DTFS_V2_ARMED_FLAG_V1553
        iorm    1,(012)
        jrst    dtfs_first_allowed_v1553

dtfs_first_verify_armed_v1554:
        ; Never authorize a second write from a volatile ARMED bit alone.
        ; This raw Type-551 read must verify a DTVOL2/DIRTY RBN0 with
        ; both exact original volume-ID halves, even after reinsertion.
        move    1,014
        move    2,015
        move    3,016
        move    4,011
        pushj   17,dtfs_intent_owned_verify_v1554
        jumpn   1,dtfs_first_refuse_v1553
        pushj   17,dtfs_first_post_io_check_v1554
        jumpn   1,dtfs_first_refuse_v1553
        jrst    dtfs_first_allowed_v1553

dtfs_first_post_io_check_v1554:
        ; Physical transfers sleep. Check both persistent saved ID halves,
        ; the original media word, and the retained MM cleanup handle
        ; AFTER the final readback but BEFORE authorizing a DATA WRITE.
        skipn   (011)
        jrst    dtfs_first_post_owner_ok_v1554
        seto    1,
        popj    17,
dtfs_first_post_owner_ok_v1554:
        move    1,(012)
        came    1,013
        jrst    dtfs_first_post_refuse_v1554
        move    1,tsfs_file_shape_base
        jumpe   1,dtfs_first_post_refuse_v1554
        add     1,010
        move    2,(1)
        came    2,015
        jrst    dtfs_first_post_refuse_v1554
        move    1,tsfs_extent_media_base
        jumpe   1,dtfs_first_post_refuse_v1554
        add     1,010
        move    2,(1)
        came    2,016
        jrst    dtfs_first_post_refuse_v1554
        setz    1,
        popj    17,
dtfs_first_post_refuse_v1554:
        seto    1,
        popj    17,
dtfs_first_allowed_v1553:
        setz    1,
        jrst    dtfs_first_done_v1553
dtfs_first_refuse_v1553:
        seto    1,
dtfs_first_done_v1553:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
