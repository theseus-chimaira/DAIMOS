; dtfs_runtime.s -- DTFS runtime and boot-patched DTC veneers.
        .text

	.globl kret_neg2
        .globl  fs_mres_vector_dispatch
        .globl  dtfs_mres_dispatch
        .globl  dtfs_lookup
        .globl  dtfs_readdir
        .globl  dtfs_stat
        .globl  dtfs_parent
        .globl  dtfs_create
        .globl  dtfs_unlink
        .globl  dtfs_rename
        .globl  dtfs_truncate
        .globl  dtfs_chmod
        .globl  dtfs_read_words
        .globl  dtfs_write_words
        .globl  dtfs_sync
        .globl  dtfs_mount_unit
        .globl  vfs_dtfs_owner_admit_v1562
        .globl  dtfs_intent_mount_seal_v1566

dtfs_mres_dispatch:
dtfs_mres_reg_dispatch:
        cain    6,026                   ; DTFS private VERIFY_REPLACE_V1531
        jrst    dtfs_verify_replacement_v1531
        ; Only the resident VFS final-unmount path requests this private
        ; operation. It MUST bypass generic vnode admission because
        ; successful v1562 quiescence intentionally denies ordinary
        ; provider operations. The seal leaf independently authenticates
        ; the exact published owner, RW policy, armed V2 and held bit.
        cain    6,027                   ; PRIVATE_SEAL_CLEAN_V1566
        jrst    dtfs_intent_mount_seal_v1566
        ; v1562: a V2 final-unmount barrier blocks provider operations
        ; which could resume after the first owner check. The internal
        ; SYNC is deliberately permitted while VFS holds this barrier.
        ; MOUNT_UNIT receives a raw physical unit, not a mounted vnode.
        cain    6,022                   ; FS_MRES_OP_MOUNT_UNIT
        jrst    dtfs_mres_dispatch_ready_v1562
        cain    6,017                   ; FS_MRES_OP_SYNC
        jrst    dtfs_mres_dispatch_ready_v1562
        push    17,1
        pushj   17,vfs_dtfs_owner_admit_v1562
        jumpn   1,dtfs_mres_dispatch_refuse_v1562
        pop     17,1
dtfs_mres_dispatch_ready_v1562:
        cain    6,025                   ; FS_MRES_OP_SPACE
        jrst    dtfs_space
        move    7,[dtfs_mres_vector]
        jrst    fs_mres_vector_dispatch
dtfs_mres_dispatch_refuse_v1562:
        pop     17,1
        jrst    kret_neg1

; Return total/used physical data words for the mounted DECtape.  Reserved
; directory/format blocks are already marked non-free in every supported
; allocation map.  The supported PDP-6 build uses native DTFS only; dormant
; TENEX/ITS compatibility remains behind disabled assembly guards.
dtfs_space:
        ; A returned/replaced physical tape must never expose stale cached
        ; allocation figures through FSINFO before identity validation.
        push    17,1
        pushj   17,dtfs_load
        jumpn   1,dtfs_space_failed_revalidate_v1522
        pop     17,1
        ldb     4,[POINT 6,1,11]        ; public mount id
        sojl    4,dtfs_space_fail
        caml    4,vfs_mount_limit       ; MM media table is boot-sized
        jrst    dtfs_space_fail
        move    5,4
        add     5,dtfs_media_base
        move    5,(5)
        hrrz    5,5
        andi    5,030                   ; personality
        setz    7,                      ; allocation-map word base
        movei   4,1                     ; native map starts at index 1
        movei   0,01101                 ; native/TENEX physical block count
        jumpe   5,dtfs_space_setup_done
        setz    4,                      ; TENEX/ITS map starts at index 0
        caie    5,020                   ; ITS has a shorter physical medium map
        jrst    dtfs_space_setup_done
        movei   7,056                   ; DTFS_ITS_NAME_WORDS
        movei   0,01067                 ; DTFS_ITS_END_BLOCK
dtfs_space_setup_done:
        move    5,0                     ; map entries remaining
        setz    6,                      ; used blocks
dtfs_space_loop:
        move    1,7
        move    2,4
        pushj   17,dtfs_owner
        jumpe   1,dtfs_space_next
        aoj     6,
dtfs_space_next:
        aoj     4,
        sojg    5,dtfs_space_loop
dtfs_space_done:
        move    1,0
        lsh     1,7                     ; 128 words/block
        move    2,6
        lsh     2,7
        popj    17,
dtfs_space_fail:
        seto    1,
        popj    17,
dtfs_space_failed_revalidate_v1522:
        pop     17,2
        jrst    dtfs_space_fail

; Accept replacement ONLY when a genuinely different full 72-bit stamped
; native DTFS medium is physically present on the poisoned unit, and its
; freshly read native directory is structurally valid. Never permit the
; same damaged medium through on an RBN0 identity match, all-zero legacy
; block or stale BCACHE tag. Under the serialized provider lease, the
; physical reads may sleep; caller rechecks all stable-uarea owners after
; the lease returns and before retiring the old slot.
;
; This does not repair, rewrite or release any data on the damaged tape.
; No persistent tombstone is necessary for old vnodes: all process and
; swap/PIPE owners must be gone, and the fresh next MOUNT revalidates
; both the new volume stamp and directory independently.
; AC1=zero-based poisoned DTFS mount slot. 0 verified, -1 refused.
dtfs_verify_replacement_v1531:
        push    17,010
        push    17,011
        move    010,1                   ; preserve slot across raw DTC I/O
        move    3,dtfs_media_base
        jumpe   3,dtfs_replace_bad_v1531
        add     3,010
        move    4,(3)
        trnn    4,0400000              ; WRITE_UNCERTAIN still set
        jrst    dtfs_replace_bad_v1531
        move    011,4
        andi    011,7                   ; original DTC physical unit
        ; Both old identity halves are needed to distinguish a genuinely
        ; different replacement; a legacy zero-ID volume cannot recover.
        move    3,tsfs_file_shape_base
        jumpe   3,dtfs_replace_bad_v1531
        add     3,010
        skipn   (3)
        jrst    dtfs_replace_bad_v1531
        move    3,tsfs_extent_media_base
        jumpe   3,dtfs_replace_bad_v1531
        add     3,010
        skipn   (3)
        jrst    dtfs_replace_bad_v1531
        setzm   dtfs_cache_mount
        pushj   17,bcache_invalidate_all

        ; Reuse the exact native directory structural admission checks
        ; from dtfs_mount_unit; do not trust a volume ID in isolation.
        ; v1558: a false-successful short Type-551 transfer must not
        ; inherit a former valid directory from this shared workspace.
        pushj   17,dtfs_workspace_clear_v1558
        move    1,011
        movei   2,0144                  ; physical native directory
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read_jump  ; actual Type-551 I/O
        jumpn   1,dtfs_replace_bad_v1531
        move    1,fs_block_workspace+0177
        came    1,[0446446632021]
        jrst    dtfs_replace_bad_v1531
        move    1,fs_block_workspace
        lsh     1,-037
        caie    1,036
        jrst    dtfs_replace_bad_v1531
        move    1,fs_block_workspace+016
        lsh     1,-025
        andi    1,037
        caie    1,036
        jrst    dtfs_replace_bad_v1531
        move    1,fs_block_workspace+0122
        and     1,[0177776]
        came    1,[0167572]
        jrst    dtfs_replace_bad_v1531

        ; Verify a complete nonlegacy DTVOL1 identity physically.
        ; Independently discard all previous directory/header words.
        ; Replacement requires a nonempty V1 stamp; all-one sentinels
        ; also expose any successful but incomplete raw read.
        pushj   17,dtfs_workspace_poison_v1558
        move    1,011
        setz    2,                      ; reserved stamped physical RBN0
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_replace_bad_v1531
        pushj   17,dtfs_identity_validate_v1522
        jumpn   1,dtfs_replace_bad_v1531
        move    4,fs_block_workspace
        came    4,[0446466575421]      ; reject all-zero legacy RBN0
        jrst    dtfs_replace_bad_v1531
        move    3,tsfs_file_shape_base
        add     3,010
        move    4,(3)
        came    4,fs_block_workspace+1 ; different high half: accepted
        jrst    dtfs_replace_ok_v1531
        move    3,tsfs_extent_media_base
        add     3,010
        move    4,(3)
        camn    4,fs_block_workspace+2 ; both halves equal: old medium
        jrst    dtfs_replace_bad_v1531
dtfs_replace_ok_v1531:
        setz    1,
        jrst    dtfs_replace_done_v1531
dtfs_replace_bad_v1531:
        seto    1,
dtfs_replace_done_v1531:
        pop     17,011
        pop     17,010
        popj    17,

        .data
dtfs_mres_vector:
        .word   dtfs_lookup,,dtfs_readdir
        .word   dtfs_stat,,dtfs_parent
        .word   0,,dtfs_create
        .word   kret_neg2,,0
        .word   dtfs_unlink,,dtfs_rename
        .word   dtfs_truncate,,dtfs_chmod
        .word   dtfs_read_words,,dtfs_write_words
        .word   dtfs_sync,,0
        .text

; DTFS is deliberately flat.  Preserve a distinct error through the syscall
; boundary so usr can report that MKDIR is unsupported rather than an
; undifferentiated filesystem failure.

; Compact five-bit allocation-map accessors.  All DTFS map indices are small
; non-negative values, so IDIVI avoids GCC's 72-bit unsigned DIV setup.
        .globl  dtfs_owner
dtfs_owner:
        idivi   2,7                     ; AC2 word index, AC3 remainder
        add     1,2                     ; AC1 map base + word index
        move    2,3
        lsh     2,2
        add     2,3                     ; remainder * 5
        add     1,dtfs_dir              ; dynamic directory-cache base
        move    1,(1)
        lsh     1,-037(2)               ; right by 31 - remainder * 5
        andi    1,037
        popj    17,

        .globl  dtfs_set_owner
dtfs_set_owner:
        move    4,3                     ; preserve new owner
        idivi   2,7                     ; AC2 word index, AC3 remainder
        add     1,2
        move    2,3
        lsh     2,2
        add     2,3                     ; remainder * 5
        movei   5,037
        sub     5,2                     ; left shift = 31 - remainder * 5
        movei   3,037
        lsh     3,0(5)
        add     1,dtfs_dir              ; dynamic directory-cache base
        andca   3,(1)
        lsh     4,0(5)
        ior     3,4
        movem   3,(1)
        popj    17,

; Compact native-directory field mutations shared by create/unlink/resize.
; All use only caller-scratch AC1..AC5 and the dynamic dtfs_dir base.
        .globl  dtfs_clear_slot
dtfs_clear_slot:
        move    2,1
        lsh     2,1
        addi    2,0123                  ; DTFS_NAME_BASE
        add     2,dtfs_dir
        setzm   (2)
        setzm   1(2)
        move    2,1
        add     2,dtfs_dir
        movni   3,2                     ; all ones except low bit
        andm    3,(2)                   ; executable bit
        andm    3,026(2)                ; high count bit at 22+slot
        popj    17,

        .globl  dtfs_set_last_words
dtfs_set_last_words:
        move    5,2                     ; preserve count bit 6
        lsh     5,-6
        andi    5,1
        move    3,1
        lsh     3,1
        addi    3,0124                  ; second name word
        add     3,dtfs_dir
        move    4,(3)
        trz     4,077
        andi    2,077
        ior     4,2
        movem   4,(3)
        move    3,1
        add     3,dtfs_dir
        move    4,026(3)
        trz     4,1
        ior     4,5
        movem   4,026(3)
        popj    17,

        .globl  dtfs_set_exec
dtfs_set_exec:
        move    3,1
        add     3,dtfs_dir
        move    4,(3)
        trz     4,1
        ior     4,2                     ; callers pass exactly 0/1
        movem   4,(3)
        popj    17,

dtfs_restore4:
        pop     17,013
dtfs_restore3:
        pop     17,012
dtfs_restore2:
        pop     17,011
dtfs_restore1:
        pop     17,010
        popj    17,

; Compact vnode predicates.  The vnode encoding is provider:6, kind/mount:12,
; index:18.  Mask provider plus local kind in one operation; mount-id and file
; index remain independent tests.

        .globl  dtfs_is_root
dtfs_is_root:
        move    2,1
        and     2,[770077000000]
        came    2,[050001000000]       ; DTFS provider, root local kind
        jrst    kret_zero
        move    2,1
        and     2,[007700000000]       ; non-zero mount id required
        jumpe   2,kret_zero
        jrst    kret_one

        .globl  dtfs_is_file
dtfs_is_file:
        move    2,1
        and     2,[770077000000]
        came    2,[050002000000]       ; DTFS provider, file local kind
        jrst    kret_zero
        move    2,1
        and     2,[007700000000]
        jumpe   2,kret_zero
        move    2,1
        andi    2,0777777
        cail    2,027                  ; ITS has 23 file slots
        jrst    kret_zero
        jrst    kret_one

; Flat DTFS parent and sync operations need only vnode classification.  The
; local predicates touch AC1/AC2 only, so AC4/AC5 can retain the original
; arguments without a compiler-generated stack frame.
        .globl  dtfs_parent
dtfs_parent:
        move    4,1                     ; original file vnode
        move    5,2                     ; vnode_t *parentp
        jumpe   5,kret_neg1
        pushj   17,dtfs_is_file
        jumpe   1,kret_neg1
        movsi   4,050001                 ; provider 5, local ROOT kind
        movem   4,(5)
        jrst    kret_zero
        .globl  dtfs_sync
dtfs_sync:
        move    4,1
        pushj   17,dtfs_is_root
        jumpn   1,dtfs_sync_ok
        move    1,4
        pushj   17,dtfs_is_file
        jumpe   1,kret_neg1
dtfs_sync_ok:
        ; Read-only/clean sync still cannot claim success for an unverified
        ; or write-uncertain volume. This also protects VFS unmount ordering.
        move    1,4
        jrst    dtfs_load
; Shared clean DTC block-cache veneers.  The key identifies the physical
; UNIT/BLOCK, not a filesystem personality, so native DTFS, TENEX, ITS, and
; TSFS can reuse the same clean copy.  MINIT patches the raw JRST slots below.
        .globl  dtfs_dtc_read
        .globl  bcache_fetch
        .globl  bcache_store
        .globl  bcache_reclaim
        .globl  bcache_invalidate_all
dtfs_dtc_read:
        move    4,2                     ; physical block fits RH
        hrl     4,1                     ; DTC key is UNIT,,BLOCK
        push    17,4
        push    17,3
        move    1,4
        move    2,3
        pushj   17,bcache_fetch
        jumpn   1,dtfs_dtc_read_hit
        hlrz    1,-1(17)                ; recover DTC unit
        hrrz    2,-1(17)                ; physical block
        move    3,(17)                  ; caller buffer
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_dtc_read_failed_v1518
        move    1,-1(17)                ; successful miss becomes clean hit
        move    2,(17)
        pushj   17,bcache_store
dtfs_dtc_read_hit:
        setz    1,
dtfs_dtc_read_done:
        sub     17,kconst_2_2
        popj    17,
dtfs_dtc_read_failed_v1518:
        ; Failed physical reads are not a media-change verdict, but the
        ; shared directory and clean block tags must not survive an
        ; unverified read.  A later access must reach real DTC I/O again.
        ; This preserves the original error in AC1; the cache leaf uses AC4.
        setzm   dtfs_cache_mount
        pushj   17,bcache_invalidate_all
        ; Mark every DTFS mount on the actual failed physical UNIT.  A later
        ; unrelated mount/cache reload must not erase the recovery barrier.
        push    17,1
        hlrz    1,-2(17)               ; saved UNIT,,BLOCK key
        movei   2,0200000              ; READ_SUSPECT in media RH
        pushj   17,dtfs_mark_unit_suspect_v1522
        pop     17,1                   ; preserve physical error exactly
        jrst    dtfs_dtc_read_done

        .globl  dtfs_dtc_read_jump
dtfs_dtc_read_jump:
        jrst    0

        .globl  dtfs_dtc_write
dtfs_dtc_write:
        ; v1555: DTVOL2 has physically admitted CLEAN only on an explicitly
        ; read-only VFS mount. Deny *all* DATA/DIR writes to its DTC unit,
        ; including internal callers which might bypass VFS readonly checks.
        ; v1561 binds the existing physical marker/retained MM owner below
        ; this unconditional RO guard. No V2 marker or DATA write occurs
        ; until exclusive CLEAN/unmount and hardware checks permit cutover.
        push    17,1
        pushj   17,dtfs_v2_ro_write_guard_v1555
        jumpn   1,dtfs_dtc_write_v2_refuse_v1555
        move    1,(17)
        ; AC2=block and AC3=buffer are live across the entire physical
        ; marker/lease operation. Save both on the private process stack:
        ; the v1552 MM allocator and controller reads may sleep/clobber ACs.
        push    17,2
        push    17,3
        move    1,-2(17)               ; original physical unit
        pushj   17,dtfs_intent_mount_gate_v1561
        jumpn   1,dtfs_dtc_write_intent_refuse_v1561
        pop     17,3
        pop     17,2
        move    1,(17)
        ; Invalidate without MM release: reclaim can refuse and can clobber
        ; the raw DTC unit/block/buffer ABI in AC1..AC3.  A successful
        ; physical write must never leave an older clean cache hit behind.
        pushj   17,bcache_invalidate_all
        pushj   17,dtfs_dtc_write_jump
        jumpe   1,dtfs_dtc_write_done_v1518
        ; A failed write may have changed part of the physical medium.
        ; Discard the shared directory identity instead of reusing it as
        ; if the metadata commit had succeeded.  Do not free the mount.
        setzm   dtfs_cache_mount
        push    17,1                   ; exact failed raw return status
        move    1,-1(17)               ; saved unit from write entry
        movei   2,0400000              ; WRITE_UNCERTAIN in media RH
        pushj   17,dtfs_mark_unit_suspect_v1522
        pop     17,1
dtfs_dtc_write_done_v1518:
        pop     17,4
        popj    17,
dtfs_dtc_write_intent_refuse_v1561:
        pop     17,3                   ; no DATA/DIR raw transfer
        pop     17,2
        jrst    dtfs_dtc_write_v2_refuse_v1555
dtfs_dtc_write_v2_refuse_v1555:
        pop     17,4                   ; release saved unit, no raw I/O
        jrst    kret_neg1

; The common DTFS write entry receives only the physical DTC unit, not
; a vnode or its VFS mount slot. Check every *published* V2 mount using
; this unit; a second mount of the same unit cannot bypass read-only.
; No persistent state, no scratch, and no mutation of the data buffer.
; AC1=unit, AC2/AC3 preserved, AC4..AC7 scratch; return AC1=0/-1.
dtfs_v2_ro_write_guard_v1555:
        move    4,vfs_mount_root_base
        jumpe   4,dtfs_v2_write_allowed_v1555
        move    5,dtfs_media_base
        jumpe   5,dtfs_v2_write_allowed_v1555
        move    6,vfs_mount_limit
        jumple  6,dtfs_v2_write_allowed_v1555
dtfs_v2_guard_next_v1555:
        move    7,(4)
        lsh     7,-036                 ; mounted vnode provider
        caie    7,5                     ; DTFS only
        jrst    dtfs_v2_guard_skip_v1555
        move    7,(5)
        trnn    7,0100000              ; DTVOL2 native format flag
        jrst    dtfs_v2_guard_skip_v1555
        andi    7,7                     ; actual physical Type-551 unit
        came    7,1
        jrst    dtfs_v2_guard_skip_v1555
        jrst    kret_neg1               ; no physical write permitted
dtfs_v2_guard_skip_v1555:
        aoj     4,
        aoj     5,
        sojg    6,dtfs_v2_guard_next_v1555
dtfs_v2_write_allowed_v1555:
        jrst    kret_zero

        .globl  dtfs_dtc_write_jump
dtfs_dtc_write_jump:
        jrst    0

; AC1=physical DTC unit, AC2=exact READ_SUSPECT or WRITE_UNCERTAIN bit.
; Scan the boot-published MM mount slab, not stale unit fields left behind
; by VFS unmount.  Root vnodes identify DTFS independently of media tags.
; No new state words, no guessed error-count threshold or lost-root HALT.
; Caller retains its original error on the stack; AC3..AC7 are scratch.
        .globl  dtfs_mark_unit_suspect_v1522
        .globl  vfs_mount_root_base
        .globl  vfs_mount_limit
        .globl  dtfs_media_base
dtfs_mark_unit_suspect_v1522:
        move    3,vfs_mount_root_base
        jumpe   3,dtfs_mark_unit_done_v1522
        move    6,dtfs_media_base
        jumpe   6,dtfs_mark_unit_done_v1522
        move    7,vfs_mount_limit
        jumple  7,dtfs_mark_unit_done_v1522
dtfs_mark_unit_next_v1522:
        move    4,(3)
        lsh     4,-036                 ; root vnode provider
        caie    4,5                    ; DTFS only
        jrst    dtfs_mark_unit_skip_v1522
        move    5,(6)
        andi    5,7
        came    5,1
        jrst    dtfs_mark_unit_skip_v1522
        iorm    2,(6)                  ; preserve owner/unit/personality
dtfs_mark_unit_skip_v1522:
        aoj     3,
        aoj     6,
        sojg    7,dtfs_mark_unit_next_v1522
dtfs_mark_unit_done_v1522:
        popj    17,

        .if DTFS_ENABLE_FOREIGN
; void dtfs_foreign_name(slot, name, its)
; Decode the two foreign SIXBIT directory words directly into the packed
; vfs_name.  NAME is already in the correct six-character format; only an
; optional dot and EXT need byte-pointer copying.
        .globl  vfs_sixbit_name_chars
        .globl  dtfs_foreign_name
dtfs_foreign_name:
        push    17,010
        move    010,2                   ; struct vfs_name *

; Clear chars plus all four packed name words.
        setzm   (010)
        movei   4,1(010)
        hrli    4,(010)
        blt     4,4(010)

; Fetch NAME and EXT.  Keep EXT temporarily in name->words[3]; a foreign name
; is at most 13 characters, so the real result never reaches that word.
        skipn   3
        jrst    dtfs_foreign_name_tenex
        lsh     1,1
        add     1,dtfs_dir
        move    5,(1)
        move    6,1(1)
        movei   7,6                     ; ITS extension limit
        jrst    dtfs_foreign_name_have_words
dtfs_foreign_name_tenex:
        add     1,dtfs_dir
        move    5,0123(1)
        move    6,0151(1)
        movei   7,3                     ; TENEX extension limit
dtfs_foreign_name_have_words:
        movem   5,1(010)
        movem   6,4(010)                ; temporary EXT word

; Cache NAME length in name->chars while finding EXT length.
        movei   1,1(010)
        movei   2,6
        pushj   17,vfs_sixbit_name_chars
        movem   1,(010)
        movei   1,4(010)
        move    2,7                     ; helper leaves AC7 untouched
        pushj   17,vfs_sixbit_name_chars
        move    4,1                     ; EXT characters
        jumpe   4,dtfs_foreign_name_finish

; Point at the first unused packed output character.
        move    5,[POINT 6,0]
        movei   6,1(010)
        hrr     5,6
        move    6,(010)
dtfs_foreign_name_seek:
        jumpe   6,dtfs_foreign_name_at_end
        ibp     5
        sojg    6,dtfs_foreign_name_seek
dtfs_foreign_name_at_end:
        skipn   (010)
        jrst    dtfs_foreign_name_copy_ext
        movei   7,016                   ; SIXBIT '.'
        idpb    7,5

; Copy only significant EXT characters from the temporary packed word.
dtfs_foreign_name_copy_ext:
        move    6,[POINT 6,0]
        movei   7,4(010)
        hrr     6,7
        move    3,4
dtfs_foreign_name_ext_loop:
        ildb    7,6
        idpb    7,5
        sojg    3,dtfs_foreign_name_ext_loop

; chars = NAME + EXT + optional dot.
        move    1,(010)
        add     1,4
        skipn   (010)
        jrst    dtfs_foreign_name_store_chars
        addi    1,1
dtfs_foreign_name_store_chars:
        movem   1,(010)
dtfs_foreign_name_finish:
        setzm   4(010)
        jrst    dtfs_restore1

; int dtfs_foreign_set_name(slot, name, its)
; Pack a VFS SIXBIT NAME[.EXT] directly into the ITS/TENEX directory words.
; AC10 keeps the NAME destination address; its LH bit 0 tags the contiguous
; ITS layout while the RH remains a normal PDP-10 index address.
        .globl  dtfs_foreign_set_name
dtfs_foreign_set_name:
        push    17,010
        jumpe   2,dtfs_foreign_set_name_fail
        skipn   3
        jrst    dtfs_foreign_set_name_tenex_setup
        move    0,1
        lsh     0,1
        move    010,dtfs_dir
        add     010,0
        tlo     010,1                   ; ITS layout tag in LH
        movei   7,6                     ; maximum EXT length
        jrst    dtfs_foreign_set_name_setup_done
dtfs_foreign_set_name_tenex_setup:
        move    010,dtfs_dir
        add     010,1
        addi    010,0123
        movei   7,3
dtfs_foreign_set_name_setup_done:
        move    3,(2)                   ; total characters
        jumple  3,dtfs_foreign_set_name_fail
        move    0,7
        addi    0,7                     ; NAME(6) + dot(1) + max EXT
        camle   3,0
        jrst    dtfs_foreign_set_name_fail
        move    4,[POINT 6,0]
        movei   0,1(2)
        hrr     4,0
        setz    5,                      ; packed NAME
        setz    6,                      ; packed EXT
        setz    1,                      ; NAME count
        setz    2,                      ; EXT count

dtfs_foreign_set_name_loop:
        ildb    0,4
        cain    0,016                   ; SIXBIT '.'
        jrst    dtfs_foreign_set_name_dot
        trne    7,0100                  ; dot already seen -> EXT
        jrst    dtfs_foreign_set_name_ext
        addi    1,1
        caile   1,6
        jrst    dtfs_foreign_set_name_fail
        lsh     5,6
        ior     5,0
        jrst    dtfs_foreign_set_name_next

dtfs_foreign_set_name_ext:
        lsh     6,6
        ior     6,0
        addi    2,1
        move    0,7
        andi    0,7
        camle   2,0
        jrst    dtfs_foreign_set_name_fail
        jrst    dtfs_foreign_set_name_next

dtfs_foreign_set_name_dot:
        trne    7,0100
        jrst    dtfs_foreign_set_name_fail
        jumpe   1,dtfs_foreign_set_name_fail
        iori    7,0100

dtfs_foreign_set_name_next:
        sojg    3,dtfs_foreign_set_name_loop
        trnn    7,0100
        jrst    dtfs_foreign_set_name_align
        jumpe   2,dtfs_foreign_set_name_fail

dtfs_foreign_set_name_align:
        movei   3,6
        sub     3,1
        imuli   3,6
        lsh     5,0(3)                  ; AC0 cannot index on PDP-6/10
        movei   3,6
        sub     3,2
        imuli   3,6
        lsh     6,0(3)
        movem   5,(010)
        tlne    010,1
        jrst    dtfs_foreign_set_name_its_ext
        movem   6,026(010)              ; TENEX EXT_BASE - NAME_BASE
        jrst    dtfs_foreign_set_name_ok
dtfs_foreign_set_name_its_ext:
        movem   6,1(010)
dtfs_foreign_set_name_ok:
        setz    1,
        jrst    dtfs_foreign_set_name_return
dtfs_foreign_set_name_fail:
        seto    1,
dtfs_foreign_set_name_return:
        jrst    dtfs_restore1

; int dtfs_scan_slot(node, name, slotp)
; Target replacement for KCC's large foreign-media scan.  AC10..AC14 retain
; the three arguments, media personality and slot across helper calls.  The
; five-word temporary vfs_name lives only on the executive stack.
        .globl  dtfs_scan_slot
        .globl  vfs_name_valid
        .globl  vfs_name_words_equal
dtfs_scan_slot:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1                  ; node
        move    011,2                  ; name or zero
        move    012,3                  ; slotp or zero
        move    1,010
        pushj   17,dtfs_personality
        move    013,1                  ; media personality
        jumpe   013,dtfs_scan_native

        jumpe   011,dtfs_scan_begin
        move    1,011
        pushj   17,vfs_name_valid
        jumpe   1,dtfs_scan_bad_name
        move    1,(011)                ; name character count
        cain    013,020                ; ITS permits NAME.EXT up to 13 chars
        jrst    dtfs_scan_its_len
        caile   1,012                  ; TENEX limit = 10 decimal
        jrst    dtfs_scan_bad_name
        jrst    dtfs_scan_begin
dtfs_scan_its_len:
        caile   1,015                  ; ITS limit = 13 decimal
        jrst    dtfs_scan_bad_name

dtfs_scan_begin:
        setz    014,                   ; slot
        add     17,kconst_5_5              ; temporary struct vfs_name
dtfs_scan_loop:
        cain    013,020
        jrst    dtfs_scan_its_empty
        move    4,014
        add     4,dtfs_dir
        skipn   0123(4)
        jrst    dtfs_scan_empty
        jrst    dtfs_scan_present
dtfs_scan_its_empty:
        move    4,014
        lsh     4,1
        add     4,dtfs_dir
        skipe   (4)
        jrst    dtfs_scan_present
        skipn   1(4)
        jrst    dtfs_scan_empty

dtfs_scan_present:
        jumpe   011,dtfs_scan_next
        move    1,014
        movei   2,-4(17)
        setz    3,
        cain    013,020
        movei   3,1
        pushj   17,dtfs_foreign_name
        move    1,-4(17)
        came    1,(011)
        jrst    dtfs_scan_next
        movei   1,-3(17)
        movei   2,1(011)
        movei   3,4
        pushj   17,vfs_name_words_equal
        jumpn   1,dtfs_scan_found
        jrst    dtfs_scan_next

dtfs_scan_empty:
        jumpn   011,dtfs_scan_next
dtfs_scan_found:
        jumpe   012,dtfs_scan_ok
        movem   014,(012)
dtfs_scan_ok:
        setz    1,
        jrst    dtfs_scan_drop

dtfs_scan_next:
        addi    014,1
        cain    013,020
        jrst    dtfs_scan_its_limit
        caige   014,026                ; native/TENEX slots = 22 decimal
        jrst    dtfs_scan_loop
        jrst    dtfs_scan_not_found
dtfs_scan_its_limit:
        caige   014,027                ; ITS slots = 23 decimal
        jrst    dtfs_scan_loop
dtfs_scan_not_found:
        seto    1,
dtfs_scan_drop:
        sub     17,kconst_5_5
        jrst    dtfs_scan_return

dtfs_scan_bad_name:
        hrroi   1,2
        jrst    dtfs_scan_return

dtfs_scan_native:
        move    1,011
        move    2,012
        pushj   17,dtfs_native_scan_slot
dtfs_scan_return:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .endif

; Compact provider lookup.  Preserve the three live arguments and one slot
; word with PUSH/POP rather than GCC's frame plus callee-save spill block.
        .globl  dtfs_load
        .if DTFS_ENABLE_FOREIGN
        .globl  dtfs_scan_slot
        .endif
        .globl  dtfs_commit
        .globl  dtfs_set_exec
        .globl  dtfs_native_scan_slot
; int dtfs_native_scan_slot(const struct vfs_name *name, unsigned int *slotp)
; Native names occupy two words; unused slots have a zero first word.
; Inline the VFS name-range check because DTFS's 11-character limit is stricter.
dtfs_native_scan_slot:
        jumpe   1,dtfs_native_scan_begin
        move    4,(1)
        jumple  4,kret_neg2
        caile   4,013                    ; DTFS_NAME_MAX_CHARS = 11
        jrst    kret_neg2
        skipe   3(1)
        jrst    kret_neg2
        skipe   4(1)
        jrst    kret_neg2
dtfs_native_scan_begin:
        setz    3,                       ; slot
        move    7,dtfs_dir
        addi    7,0123                   ; DTFS_NAME_BASE
dtfs_native_scan_loop:
        jumpe   1,dtfs_native_scan_empty
        move    5,(7)
        came    5,1(1)
        jrst    dtfs_native_scan_next
        move    5,1(7)
        andcmi  5,077                    ; ignore native tail-count bits
        move    6,2(1)
        andcmi  6,077
        came    5,6
        jrst    dtfs_native_scan_next
        jrst    dtfs_native_scan_match
dtfs_native_scan_empty:
        skipn   (7)
        jrst    dtfs_native_scan_match
dtfs_native_scan_next:
        addi    7,2
        addi    3,1
        caige   3,026                    ; DTFS_FILE_SLOTS = 22
        jrst    dtfs_native_scan_loop
        jrst    kret_neg1
dtfs_native_scan_match:
        jumpe   2,kret_zero
        movem   3,(2)
        jrst    kret_zero

        .globl  kret_zero
        .globl  kret_neg1

        .globl  dtfs_mount_unit
        .globl  vfs_mount_prevalidated
        .globl  vfs_current_owner
; int dtfs_mount_unit(unit, target, flags, rootp)
; Cold mount-control path.  AC10=media, AC11=target, AC12=flags/mount id,
; AC13=rootp.  One stack local holds the prevalidated mounted root vnode.
dtfs_mount_unit:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; unit, later packed media
        move    011,2                   ; target vnode
        move    012,3                   ; flags
        move    013,4                   ; rootp

        ; Unsigned unit <= 7, non-null output, and only RO/type flag bits.
        move    1,010
        tlc     1,0400000
        camg    1,[0400000000007]
        skipn   013
        jrst    dtfs_mount_unit_fail
        move    1,012
        and     1,[-032]
        jumpn   1,dtfs_mount_unit_fail

        ; Preserve the C feature gates: unsupported foreign personalities
        ; must be rejected even though their flag values are syntactically valid.
        move    1,012
        andi    1,030
        cain    1,010                   ; SYS_DTFS_TYPE_NATIVE
        jrst    dtfs_mount_unit_media_ready
        .if DTFS_ENABLE_TENEX
        cain    1,020
        jrst    dtfs_mount_unit_tenex
        .endif
        .if DTFS_ENABLE_ITS
        cain    1,030
        jrst    dtfs_mount_unit_its
        .endif
        jrst    dtfs_mount_unit_fail
        .if DTFS_ENABLE_TENEX
dtfs_mount_unit_tenex:
        iori    010,010                 ; DTFS_MEDIA_TENEX
        jrst    dtfs_mount_unit_media_ready
        .endif
        .if DTFS_ENABLE_ITS
dtfs_mount_unit_its:
        iori    010,020                 ; DTFS_MEDIA_ITS
        .endif
dtfs_mount_unit_media_ready:
        ; V1519: the short-lived userspace probe is not a trust boundary.
        ; A direct MOUNT_UNIT caller can bypass it, and the medium can be
        ; changed after the probe.  Verify native directory invariants from
        ; a FRESH PHYSICAL read before publishing a VFS mount slot.  An
        ; ordinary BCACHE hit is not proof of the current tape contents.
        ; These are format checks, NOT unique-volume identity: two distinct
        ; native tapes can both pass them.  Replacement of an OPEN mount
        ; still requires a separate checked media-identity protocol.
        .if DTFS_ENABLE_FOREIGN
        move    1,012
        andi    1,030
        caie    1,010                   ; native DTFS only
        jrst    dtfs_mount_unit_foreign_identity_v1520
        .endif
        pushj   17,bcache_invalidate_all
        setzm   dtfs_cache_mount
        ; v1558: physical DTC success does not necessarily prove 128
        ; words transferred. Wipe the shared workspace before admission
        ; of BOTH the directory and independently checked RBN0.
        ; The native directory must contain nonzero structural metadata;
        ; clearing stale words makes false-successful short reads invalid.
        pushj   17,dtfs_workspace_clear_v1558
        move    1,010
        andi    1,7                     ; physical DTC unit
        movei   2,0144                  ; native directory block
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read_jump  ; bypass all old clean tags
        jumpn   1,dtfs_mount_unit_fail
        move    1,fs_block_workspace+0177
        came    1,[0446446632021]      ; native directory magic
        jrst    dtfs_mount_unit_fail
        move    1,fs_block_workspace
        lsh     1,-037                  ; owner of physical block zero
        caie    1,036                   ; reserved
        jrst    dtfs_mount_unit_fail
        ; The native owner map stores physical RBN 0144 at dir[14]
        ; (C decimal index 14 = PDP-6 assembly octal offset 016).
        ; Octal +014 would inspect word 12 and reject a valid MKDTFS tape.
        move    1,fs_block_workspace+016
        lsh     1,-025                  ; owner of physical block 0144
        andi    1,037
        caie    1,036                   ; directory is reserved
        jrst    dtfs_mount_unit_fail
        move    1,fs_block_workspace+0122
        and     1,[0177776]            ; 3 final five-bit owner tags
        came    1,[0167572]            ; each must be 035 (native tag)
        jrst    dtfs_mount_unit_fail

        ; V1520: native RBN zero is reserved and previously all zero.
        ; Read it PHYSICALLY; a clean unit/block tag from an earlier tape
        ; is not evidence of the currently installed volume.  All-zero
        ; remains accepted as legacy format, but provides NO identity
        ; for reattachment of live open files.  A nonempty corrupt record
        ; is always fatal to this mount admission (not to KCORE).
        ; A complete all-zero RBN0 is a VALID legacy format. Initializing
        ; to zero here would *accept* a zero-byte READ falsely reporting
        ; success as a genuine legacy volume. Fill every word with a
        ; nonzero poison; a complete legitimate legacy READ overwrites it.
        pushj   17,dtfs_workspace_poison_v1558
        move    1,010
        andi    1,7
        setz    2,                      ; native identity RBN 0
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read_jump
        jumpn   1,dtfs_mount_unit_fail
        ; v1555: the default DTVOL1 and unstamped legacy rules continue
        ; unchanged. DTVOL2 is admitted only as a fully sealed CLEAN
        ; physical block, and only when VFS flags request READ-ONLY.
        ; Dirty or malformed DTVOL2 never becomes a writable mount.
        move    1,fs_block_workspace
        came    1,[0446466575422]       ; exact SIXBIT DTVOL2
        jrst    dtfs_mount_legacy_identity_v1555
        move    1,012
        trnn    1,1                     ; VFS_MOUNT_RDONLY explicitly set
        jrst    dtfs_mount_unit_fail
        movei   1,fs_block_workspace
        pushj   17,dtfs_intent_validate_v1550
        jumpn   1,dtfs_mount_unit_fail ; DIRTY and INVALID both refused
        ; v1557: a physical V2 volume must have one unambiguous DTFS
        ; mount owner. Reject even an older V1 mount on this DTC unit.
        move    1,010
        andi    1,7
        movei   2,1                     ; candidate DTVOL2 mount
        pushj   17,dtfs_v2_mount_exclusive_v1557
        jumpn   1,dtfs_mount_unit_fail
        iori    010,0100000            ; runtime FORMAT_V2 only after read
        jrst    dtfs_mount_save_id_v1555
dtfs_mount_legacy_identity_v1555:
        pushj   17,dtfs_identity_validate_v1522
        jumpn   1,dtfs_mount_unit_fail
        ; Also forbid adding a V1 mount to a unit already claimed by V2.
        ; Preserve the previous V1/V1 compatibility contract.
        move    1,010
        andi    1,7
        setz    2,                      ; candidate V1 / zero legacy
        pushj   17,dtfs_v2_mount_exclusive_v1557
        jumpn   1,dtfs_mount_unit_fail
dtfs_mount_save_id_v1555:
        ; Keep both 36-bit halves on our private stack across the VFS
        ; publication call.  The shared physical workspace may be reused
        ; by another provider while that call is in progress.
        push    17,fs_block_workspace+1
        push    17,fs_block_workspace+2
        jrst    dtfs_mount_unit_format_done_v1519
        .if DTFS_ENABLE_FOREIGN
dtfs_mount_unit_foreign_identity_v1520:
        push    17,[0]                 ; foreign media are not stamped
        push    17,[0]
dtfs_mount_unit_format_done_v1519:
        .else
dtfs_mount_unit_format_done_v1519:
        .endif

        push    17,[0]                  ; root local
        movei   1,(17)                  ; sixth argument: &root
        push    17,1
        move    1,012
        andi    1,1                     ; fifth argument: read-only bit
        push    17,1
        move    1,011
        movei   2,5                     ; DTFS_PROVIDER
        movei   3,1                     ; DTFS_KIND_ROOT
        setz    4,                      ; index 0
        pushj   17,vfs_mount_prevalidated
        sub     17,kconst_2_2
        jumpn   1,dtfs_mount_unit_fail_local

        hlrz    012,(17)
        lsh     012,-6
        andi    012,077                 ; mount id
        pushj   17,vfs_current_owner
        lsh     1,022                   ; owner18 into LH
        move    2,010
        ior     2,1                     ; media | owner18
        move    1,012
        pushj   17,dtfs_patch_media
        ; The MM-backed SHAPE and EXTENT words are otherwise TSFS-only.
        ; A DTFS mount occupies a disjoint VFS slot from every TSFS set.
        ; Reuse those two per-slot words for its 72-bit identity; legacy
        ; mounts keep both zero and are NOT eligible for trusted reuse.
        move    3,012
        subi    3,1
        move    2,tsfs_file_shape_base
        add     2,3
        move    1,-2(17)               ; high half of volume identifier
        movem   1,(2)
        move    2,tsfs_extent_media_base
        add     2,3
        move    1,-1(17)               ; low half
        movem   1,(2)
        setzm   dtfs_cache_mount
        move    1,(17)
        movem   1,(013)
        setz    1,
        jrst    dtfs_mount_unit_done_local

dtfs_mount_unit_fail_local:
        seto    1,
dtfs_mount_unit_done_local:
        sub     17,kconst_3_3          ; root local and two ID halves
        jrst    dtfs_restore4
dtfs_mount_unit_fail:
        seto    1,
        jrst    dtfs_restore4

; v1558: initialize one complete Type-551 transfer buffer before every
; independent raw admission read. Nonzero poison is essential for RBN0:
; a real all-zero legacy header is valid, so prezeroed memory combined
; with a false-successful zero-byte DMA is otherwise indistinguishable
; from a complete true legacy transfer. Directory admission can use zero
; fill because its structural header is required to be nonzero.
; Scratch AC4..AC6, preserves all mount arguments and saved ID halves.
dtfs_workspace_clear_v1558:
        setz    6,
        jrst    dtfs_workspace_fill_v1558
dtfs_workspace_poison_v1558:
        seto    6,
dtfs_workspace_fill_v1558:
        movei   4,fs_block_workspace
        movei   5,0200                 ; 128 PDP-6 data words
dtfs_workspace_clear_loop_v1558:
        movem   6,(4)
        aoj     4,
        sojg    5,dtfs_workspace_clear_loop_v1558
        popj    17,

; Shared LEGACY native identity parser for DTVOL1/zero mount admission and
; post-error revalidation of an existing legacy mount. The v1555 mount
; path selects the separate full DTVOL2/CLEAN verifier first, and forces
; RO before publishing its format bit. Requires a PHYSICAL RBN0 read into
; fs_block_workspace by the caller. Returns zero for structurally valid
; DTVOL1 or all-zero legacy; -1 for other nonempty records, including V2.
; Legacy recovery additionally requires a nonzero previously saved 72-bit
; ID and an exact match of both halves. No added permanent scratch.
dtfs_identity_validate_v1522:
        movei   1,fs_block_workspace
        movei   2,0200                 ; full 128-word reserved block
        setz    3,
dtfs_mount_identity_nonzero_loop_v1520:
        ior     3,(1)
        aoj     1,
        sojg    2,dtfs_mount_identity_nonzero_loop_v1520
        jumpe   3,dtfs_mount_identity_valid_v1520
        move    1,fs_block_workspace
        came    1,[0446466575421]      ; SIXBIT DTVOL1
        jrst    dtfs_identity_invalid_v1522
        skipn   fs_block_workspace+1
        jrst    dtfs_identity_invalid_v1522
        skipn   fs_block_workspace+2
        jrst    dtfs_identity_invalid_v1522
        xor     1,fs_block_workspace+1
        xor     1,fs_block_workspace+2
        setcm   1,1                     ; 36-bit complement checksum
        came    1,fs_block_workspace+3
        jrst    dtfs_identity_invalid_v1522
        movei   1,fs_block_workspace+4
        movei   2,0174                 ; 124 zero padding words
dtfs_mount_identity_padding_loop_v1520:
        skipn   (1)
        jrst    dtfs_mount_identity_padding_next_v1520
        jrst    dtfs_identity_invalid_v1522
dtfs_mount_identity_padding_next_v1520:
        aoj     1,
        sojg    2,dtfs_mount_identity_padding_loop_v1520
dtfs_mount_identity_valid_v1520:
        setz    1,
        popj    17,
dtfs_identity_invalid_v1522:
        seto    1,
        popj    17,

dtfs_lookup:
        push    17,010
        push    17,011
        push    17,012
        push    17,[0]
        move    010,1                   ; dir
        move    011,2                   ; name
        move    012,3                   ; nodep
        pushj   17,dtfs_is_root
        jumpe   1,dtfs_lookup_fail
        jumpe   012,dtfs_lookup_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_lookup_fail
        .if DTFS_ENABLE_FOREIGN
        move    1,010
        move    2,011
        movei   3,(17)
        pushj   17,dtfs_scan_slot
        .else
        move    1,011
        movei   2,(17)
        pushj   17,dtfs_native_scan_slot
        .endif
        jumpn   1,dtfs_lookup_fail
        hrrz    010,(17)                ; slot index
        tlo     010,050002              ; provider 5, local FILE kind
        movem   010,(012)
        setz    1,
        jrst    dtfs_lookup_return

dtfs_lookup_fail:
        seto    1,
dtfs_lookup_return:
        pop     17,0                    ; slot scratch
        pop     17,012
        jrst    dtfs_restore2

; Native chmod only.  Foreign personalities are read-only at this provider
; entry.  Keep NODE/MODE in callee-saved ACs across C helpers.
        .globl  dtfs_create
        .globl  fs_zero_block_workspace
        .globl  fs_block_workspace
; int dtfs_create(dir, name, mode, nodep)
; Transactional metadata creation.  AC10..AC15 retain C arguments, selected
; slot and personality; one stack word is reused for the TENEX data block.
dtfs_create:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,[0]
        move    010,1                   ; dir
        move    011,2                   ; name
        move    012,3                   ; mode
        move    013,4                   ; nodep

        pushj   17,dtfs_is_root
        jumpe   1,dtfs_create_fail
        skipn   013
        jrst    dtfs_create_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_create_fail
        move    1,010
        move    2,011
        setz    3,
        pushj   17,dtfs_scan_slot
        aoje    1,dtfs_create_find_slot ; exactly -1 means name absent
        jrst    dtfs_create_fail

dtfs_create_find_slot:
        .if DTFS_ENABLE_FOREIGN
        move    1,010
        pushj   17,dtfs_personality
        move    015,1
        .else
        setz    015,
        .endif
        move    1,010
        setz    2,
        movei   3,(17)
        pushj   17,dtfs_scan_slot
        jumpn   1,dtfs_create_fail
        move    014,(17)                ; free directory slot

        .if DTFS_ENABLE_TENEX
        cain    015,010
        jrst    dtfs_create_tenex
        .endif
        .if DTFS_ENABLE_ITS
        cain    015,020
        jrst    dtfs_create_its
        .endif
        jrst    dtfs_create_native

        .if DTFS_ENABLE_TENEX
dtfs_create_tenex:
        movei   1,1
        movei   2,1
        movei   3,(17)
        pushj   17,dtfs_find_free_block
        jumpn   1,dtfs_create_fail
        move    1,014
        move    2,011
        setz    3,
        pushj   17,dtfs_foreign_set_name
        jumpn   1,dtfs_create_fail
        pushj   17,fs_zero_block_workspace
        move    2,(17)
        lsh     2,010                   ; DTFS_HEADER(0, block, 0)
        movem   2,fs_block_workspace
        move    1,010
        pushj   17,dtfs_unit
        move    2,(17)
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_write
        jumpn   1,dtfs_create_tenex_clear

        setz    1,
        move    2,(17)
        subi    2,1
        move    3,014
        addi    3,1
        pushj   17,dtfs_set_owner
        move    1,010
        pushj   17,dtfs_commit
        jumpe   1,dtfs_create_success
        setz    1,
        move    2,(17)
        subi    2,1
        setz    3,
        pushj   17,dtfs_set_owner
dtfs_create_tenex_clear:
        move    4,014
        add     4,dtfs_dir
        setzm   0123(4)                 ; TENEX NAME
        setzm   0151(4)                 ; TENEX EXT
        jrst    dtfs_create_fail
        .endif

        .if DTFS_ENABLE_ITS
dtfs_create_its:
        move    1,014
        move    2,011
        movei   3,1
        pushj   17,dtfs_foreign_set_name
        jumpn   1,dtfs_create_fail
        move    1,010
        pushj   17,dtfs_commit
        jumpe   1,dtfs_create_success
        move    4,014
        lsh     4,1
        add     4,dtfs_dir
        setzm   (4)
        setzm   1(4)
        jrst    dtfs_create_fail
        .endif

dtfs_create_native:
        move    1,014
        move    2,011
        pushj   17,dtfs_set_name
        move    1,014
        setz    2,
        pushj   17,dtfs_set_last_words
        move    1,014
        move    2,012
        andi    2,0111
        jumpe   2,dtfs_create_native_exec
        movei   2,1
dtfs_create_native_exec:
        pushj   17,dtfs_set_exec
        move    1,010
        pushj   17,dtfs_commit
        jumpe   1,dtfs_create_success
        move    1,014
        pushj   17,dtfs_clear_slot
        jrst    dtfs_create_fail

dtfs_create_success:
        move    2,014
        tlo     2,050002                ; provider-local DTFS FILE vnode
        movem   2,(013)
        setz    1,
        jrst    dtfs_create_done
dtfs_create_fail:
        seto    1,
dtfs_create_done:
        sub     17,kconst_1_1
        pop     17,015
        pop     17,014
        jrst    dtfs_restore4

        .globl  dtfs_unlink
; int dtfs_unlink(dir, name)
; AC10=dir, AC11=name, AC12=slot, AC13=personality, AC14=file vnode,
; AC15=remaining TENEX blocks.  Two stack locals hold next/current block.
dtfs_unlink:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        add     17,kconst_2_2
        move    010,1
        move    011,2

        pushj   17,dtfs_is_root
        jumpe   1,dtfs_unlink_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_unlink_fail
        move    1,010
        move    2,011
        movei   3,-1(17)               ; slot local
        pushj   17,dtfs_scan_slot
        jumpn   1,dtfs_unlink_fail
        move    012,-1(17)

        .if DTFS_ENABLE_FOREIGN
        move    1,010
        pushj   17,dtfs_personality
        move    013,1
        .else
        setz    013,
        .endif

        ; Convert mounted ROOT vnode to FILE while preserving provider/mount.
        move    014,010
        tlc     014,000003              ; ROOT(1) XOR 3 = FILE(2)
        hrr     014,012

        .if DTFS_ENABLE_TENEX
        cain    013,010
        jrst    dtfs_unlink_tenex
        .endif
        jrst    dtfs_unlink_regular

        .if DTFS_ENABLE_TENEX
dtfs_unlink_tenex:
        move    1,014
        move    2,012
        movei   3,(17)                  ; first block -> next local
        pushj   17,dtfs_block_info
        move    015,1                   ; remaining block count
        jumpe   015,dtfs_unlink_fail
        skipn   (17)
        jrst    dtfs_unlink_fail

dtfs_unlink_tenex_loop:
        move    4,(17)
        movem   4,-1(17)                ; current block
        jumpe   4,dtfs_unlink_corrupt
        caile   4,01101                 ; DTFS_LAST_BLOCK
        jrst    dtfs_unlink_corrupt
        setz    1,
        move    2,4
        subi    2,1
        pushj   17,dtfs_owner
        move    3,012
        addi    3,1
        came    1,3
        jrst    dtfs_unlink_corrupt
        move    1,010
        pushj   17,dtfs_unit
        move    2,-1(17)
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,dtfs_unlink_corrupt
        hlrz    4,fs_block_workspace
        andi    4,01777                 ; DTFS_HDR_NEXT
        movem   4,(17)
        setz    1,
        move    2,-1(17)
        subi    2,1
        setz    3,
        pushj   17,dtfs_set_owner
        sojg    015,dtfs_unlink_tenex_loop
        skipn   (17)
        jrst    dtfs_unlink_tenex_clear
dtfs_unlink_corrupt:
        setzm   dtfs_cache_mount
        jrst    dtfs_unlink_fail

dtfs_unlink_tenex_clear:
        move    4,012
        add     4,dtfs_dir
        setzm   0123(4)
        setzm   0151(4)
        move    1,010
        pushj   17,dtfs_commit
        jumpe   1,dtfs_unlink_success
        setzm   dtfs_cache_mount
        jrst    dtfs_unlink_fail
        .endif

dtfs_unlink_regular:
        move    1,014
        setz    2,
        pushj   17,dtfs_resize
        jumpn   1,dtfs_unlink_fail
        .if DTFS_ENABLE_ITS
        cain    013,020
        jrst    dtfs_unlink_its_clear
        .endif
        move    1,012
        pushj   17,dtfs_clear_slot
        jrst    dtfs_unlink_commit
        .if DTFS_ENABLE_ITS
dtfs_unlink_its_clear:
        move    4,012
        lsh     4,1
        add     4,dtfs_dir
        setzm   (4)
        setzm   1(4)
        .endif

dtfs_unlink_commit:
        move    1,010
        pushj   17,dtfs_commit
        jrst    dtfs_unlink_done
dtfs_unlink_success:
        setz    1,
        jrst    dtfs_unlink_done
dtfs_unlink_fail:
        seto    1,
dtfs_unlink_done:
        sub     17,kconst_2_2
        pop     17,015
        pop     17,014
        jrst    dtfs_restore4

        .globl  dtfs_rename
; int dtfs_rename(olddir, oldname, newdir, newname)
; Cold metadata path.  Four callee-saved ACs retain the arguments and one
; stack word retains the source slot while helper calls use caller-scratch ACs.
dtfs_rename:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,[0]                  ; source slot
        move    010,1                   ; olddir
        move    011,2                   ; oldname
        move    012,3                   ; newdir
        move    013,4                   ; newname

        pushj   17,dtfs_is_root
        jumpe   1,dtfs_rename_fail
        move    1,012
        pushj   17,dtfs_is_root
        jumpe   1,dtfs_rename_fail
        move    1,010
        xor     1,012
        and     1,[007700000000]        ; mount ids must match
        jumpn   1,dtfs_rename_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_rename_fail

        move    1,010
        move    2,011
        movei   3,(17)
        pushj   17,dtfs_scan_slot
        jumpn   1,dtfs_rename_fail
        move    1,010
        move    2,013
        setz    3,
        pushj   17,dtfs_scan_slot
        aoje    1,dtfs_rename_name      ; exactly -1 means destination absent
        jrst    dtfs_rename_fail

dtfs_rename_name:
        .if DTFS_ENABLE_FOREIGN
        move    1,010
        pushj   17,dtfs_personality
        jumpe   1,dtfs_rename_native
        move    4,1
        move    1,(17)                  ; source slot
        move    2,013                   ; new name
        setz    3,
        cain    4,020                   ; ITS personality flag to helper
        movei   3,1
        pushj   17,dtfs_foreign_set_name
        jumpn   1,dtfs_rename_fail
        jrst    dtfs_rename_commit
        .endif

dtfs_rename_native:
        move    1,(17)
        move    2,013
        pushj   17,dtfs_set_name

dtfs_rename_commit:
        move    1,010
        pushj   17,dtfs_commit
        jrst    dtfs_rename_done
dtfs_rename_fail:
        seto    1,
dtfs_rename_done:
        sub     17,kconst_1_1
        jrst    dtfs_restore4

dtfs_chmod:
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        caile   011,07777                 ; reject CHOWN/UTIME private commands
        jrst    dtfs_chmod_fail
        pushj   17,dtfs_is_file
        jumpe   1,dtfs_chmod_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_chmod_fail
        .if DTFS_ENABLE_FOREIGN
        move    1,010
        pushj   17,dtfs_personality
        jumpn   1,dtfs_chmod_fail
        .endif
        hrrz    1,010
        move    2,011
        andi    2,0111
        jumpe   2,dtfs_chmod_have_exec
        movei   2,1
dtfs_chmod_have_exec:
        pushj   17,dtfs_set_exec
        move    1,010
        pop     17,011
        pop     17,010
        jrst    dtfs_commit

dtfs_chmod_fail:
        pop     17,011
        pop     17,010
        jrst    kret_neg1

.if DTFS_ENABLE_FOREIGN
; The optional TENEX/ITS personality is already encoded in the MM-backed
; media word.  Do not maintain a separate four-instruction self-modifying XCT
; table: it both truncates mount IDs and wastes permanent resident words.
; Lookup is cold relative to tape I/O.  The published table and limit remain
; the single authoritative source of the media personality for all 26 slots.

        .globl  dtfs_patch_media
dtfs_patch_media:
        move    3,1
        subi    3,1
        add     3,dtfs_media_base
        movem   2,(3)
        pushj   17,bcache_reclaim       ; source id may refer to new media
        popj    17,

        .globl  dtfs_personality
dtfs_personality:
        ldb     2,[POINT 6,1,11]       ; public mount ID
        sojl    2,dtfs_personality_bad
        caml    2,vfs_mount_limit
        jrst    dtfs_personality_bad
        add     2,dtfs_media_base
        move    1,(2)
        andi    1,030                 ; native 0, TENEX 010, ITS 020
        popj    17,
dtfs_personality_bad:
        setz    1,                    ; invalid mount cannot acquire mode
        popj    17,

 ; Mount ownership is VFS policy, never DECtape media metadata.  The high
; half of each dtfs_media word carries compact UID9,GID9 while its low half
; remains the existing unit/personality value.

; void dtfs_stat_owner(vnode, struct vfs_stat *st)
; dtfs_media LH is compact UID9,,GID9 for the mount.
        .globl  dtfs_stat_owner
dtfs_stat_owner:
        ldb     3,[POINT 6,1,11]        ; public mount ID 1..capacity
        subi    3,1
        move    4,3
        add     4,dtfs_media_base
        hlrz    4,(4)                  ; owner18
        move    3,4
        lsh     3,-011
        andi    3,0777
        movem   3,4(2)                  ; uid
        andi    4,0777
        movem   4,5(2)                  ; gid
        popj    17,

; Shared cached directory loader.  Mount-time userspace validation has already
; established the media personality; runtime only reloads the selected
; directory block when another DTFS/TSFS mount displaced the cache.
        .globl  dtfs_cache_mount
        .globl  dtfs_media_base

dtfs_load:
        push    17,010
        ldb     010,[POINT 6,1,11]      ; mount id
        move    1,010
        pushj   17,dtfs_identity_revalidate_v1522
        jumpn   1,dtfs_load_fail
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_ok
        move    4,010
        subi    4,1
        add     4,dtfs_media_base
        move    4,(4)                   ; unit + personality
        move    1,4
        andi    1,7                     ; unit
        movei   2,0144                  ; native/TENEX directory
.if DTFS_ENABLE_ITS
        trne    4,020
        movei   2,0100                  ; ITS directory
.endif
        move    3,dtfs_dir
        pushj   17,dtfs_dtc_read
        jumpn   1,dtfs_load_fail
        movem   010,dtfs_cache_mount

dtfs_load_ok:
        pop     17,010
        jrst    kret_zero

dtfs_load_fail:
        pop     17,010
        jrst    kret_neg1

.else
; Native-only build: packed media contains only the unit in RH; LH still stores
; the mount owner used by VFS stat/access policy.
        .globl  dtfs_stat_owner
dtfs_stat_owner:
        ldb     3,[POINT 6,1,11]
        subi    3,1
        move    4,3
        add     4,dtfs_media_base
        hlrz    4,(4)
        move    3,4
        lsh     3,-011
        andi    3,0777
        movem   3,4(2)
        andi    4,0777
        movem   4,5(2)
        popj    17,

        .globl  dtfs_personality
dtfs_personality:
        setz    1,
        popj    17,

        .globl  dtfs_scan_slot
dtfs_scan_slot:
        move    1,2
        move    2,3
        jrst    dtfs_native_scan_slot

        .globl  dtfs_patch_media
dtfs_patch_media:
        move    3,1
        subi    3,1
        add     3,dtfs_media_base
        movem   2,(3)
        pushj   17,bcache_reclaim
        popj    17,

        .globl  dtfs_cache_mount
        .globl  dtfs_media_base

dtfs_load:
        push    17,010
        ldb     010,[POINT 6,1,11]
        move    1,010
        pushj   17,dtfs_identity_revalidate_v1522
        jumpn   1,dtfs_load_native_fail
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_native_ok
        move    1,010
        subi    1,1
        add     1,dtfs_media_base
        move    1,(1)
        andi    1,7
        movei   2,0144
        move    3,dtfs_dir
        pushj   17,dtfs_dtc_read
        jumpn   1,dtfs_load_native_fail
        movem   010,dtfs_cache_mount
dtfs_load_native_ok:
        pop     17,010
        jrst    kret_zero
dtfs_load_native_fail:
        pop     17,010
        jrst    kret_neg1

.endif

; AC1=public DTFS mount ID. A read fault in a mounted physical unit
; marks that mount in an unused bit of its existing media word. On its
; next operation, read RBN0 PHYSICALLY before allowing any cached vnode,
; clean BCACHE, directory or FILE reuse. A complete DTVOL1 record,
; or a complete DTVOL2/CLEAN record for a V2 read-only mount, matching
; BOTH original saved ID halves may clear only READ_SUSPECT.
; A write failure may have partially changed directory/data: preserve
; WRITE_UNCERTAIN and refuse all old operations, even on an ID match,
; until an explicit checked recovery path is implemented. Neither
; condition is evidence that a critical root medium vanished.
dtfs_identity_revalidate_v1522:
        sojl    1,dtfs_identity_revalidate_fail_v1522
        caml    1,vfs_mount_limit
        jrst    dtfs_identity_revalidate_fail_v1522
        move    6,1                     ; zero-based mount slot
        add     1,dtfs_media_base
        move    5,(1)
        trne    5,0400000              ; WRITE_UNCERTAIN
        jrst    dtfs_identity_revalidate_fail_v1522
        trnn    5,0200000              ; READ_SUSPECT
        jrst    kret_zero
        ; BCACHE slot 0 IS fs_block_workspace. A raw RBN0 read overwrites
        ; that storage without going through bcache_fetch's tag tracking.
        ; Invalidate the workspace tag first: otherwise a later TSFS
        ; metadata lookup can hit a stale tag and interpret the freshly
        ; read DTVOL1 record as a cached TSFS file record. Slot 1 remains
        ; a valid independent clean copy; no global cache flush needed.
        pushj   17,bcache_workspace_invalidate
        ; v1556: clear stale words before I/O. A short transfer reporting
        ; success must not reuse an earlier complete RBN0 from BCACHE.
        ; Raw DTC may clobber AC4..AC7; retain BOTH the original media
        ; word and slot on the process stack across the sleeping read.
        push    17,6
        push    17,5
        movei   4,fs_block_workspace
        movei   7,0200
dtfs_revalidate_clear_v1556:
        setzm   (4)
        aoj     4,
        sojg    7,dtfs_revalidate_clear_v1556
        move    1,(17)
        andi    1,7                     ; physical DTC selector
        setz    2,                      ; reserved native RBN0
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read_jump  ; no stale BCACHE hit
        jumpn   1,dtfs_identity_revalidate_pop_fail_v1522
        move    5,(17)                  ; original media, not raw clobber
        trnn    5,0100000              ; FORMAT_V2 on this mount
        jrst    dtfs_revalidate_v1_v1556
        movei   1,fs_block_workspace
        pushj   17,dtfs_intent_validate_v1550
        jumpn   1,dtfs_identity_revalidate_pop_fail_v1522
        ; Only DTVOL2/CLEAN is mounted read-only. A DIRTY record, even
        ; with the original identity, must not be treated as recovered.
        jrst    dtfs_revalidate_valid_v1556
dtfs_revalidate_v1_v1556:
        pushj   17,dtfs_identity_validate_v1522
        jumpn   1,dtfs_identity_revalidate_pop_fail_v1522
dtfs_revalidate_valid_v1556:
        pop     17,5                    ; original mount media word
        pop     17,6
        move    4,tsfs_file_shape_base
        jumpe   4,dtfs_identity_revalidate_fail_v1522
        add     4,6
        move    4,(4)
        jumpe   4,dtfs_identity_revalidate_fail_v1522
        came    4,fs_block_workspace+1
        jrst    dtfs_identity_revalidate_fail_v1522
        move    4,tsfs_extent_media_base
        jumpe   4,dtfs_identity_revalidate_fail_v1522
        add     4,6
        move    4,(4)
        jumpe   4,dtfs_identity_revalidate_fail_v1522
        came    4,fs_block_workspace+2
        jrst    dtfs_identity_revalidate_fail_v1522
        move    4,dtfs_media_base
        add     4,6
        move    3,(4)
        came    3,5                     ; mount changed during sleeping I/O
        jrst    dtfs_identity_revalidate_fail_v1522
        trnn    5,0100000
        jrst    dtfs_revalidate_clear_suspect_v1556
        ; A V2 recovery is valid only for the original published native
        ; DTFS root and a still-read-only VFS slot. Reject stale slots,
        ; changed providers and a hypothetical unsafe RW state transition.
        move    1,vfs_mount_root_base
        jumpe   1,dtfs_identity_revalidate_fail_v1522
        add     1,6
        move    2,(1)
        ldb     3,[POINT 6,2,5]
        caie    3,5                     ; exact DTFS provider
        jrst    dtfs_identity_revalidate_fail_v1522
        ldb     3,[POINT 6,2,11]
        movei   7,1(6)
        came    3,7                     ; original published mount slot
        jrst    dtfs_identity_revalidate_fail_v1522
        move    1,vfs_mount_policy
        movn    2,6
        lsh     1,0(2)
        trnn    1,1                     ; VFS_MOUNT_RDONLY still set
        jrst    dtfs_identity_revalidate_fail_v1522
dtfs_revalidate_clear_suspect_v1556:
        movei   3,0200000
        andcam  3,(4)                   ; clear ONLY the read-fault marker
        jrst    kret_zero
dtfs_identity_revalidate_pop_fail_v1522:
        pop     17,5
        pop     17,6
dtfs_identity_revalidate_fail_v1522:
        jrst    kret_neg1


.if DTFS_ENABLE_FOREIGN
; Compact directory enumeration.  Resolve the personality once and scan using
; caller-clobbered ACs; no state must survive a helper call until a matching
; entry has already been selected.
; int dtfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
dtfs_readdir:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; dir
        move    011,2                   ; requested visible entry
        move    012,3                   ; result
        pushj   17,dtfs_is_root
        jumpe   1,dtfs_readdir_fail
        jumpe   012,dtfs_readdir_fail
        move    1,010
        pushj   17,dtfs_load
        jumpn   1,dtfs_readdir_fail
        move    1,010
        pushj   17,dtfs_personality
        move    013,1                   ; 0 native, 010 TENEX, 020 ITS
        setzb   4,5                     ; slot, seen

dtfs_readdir_loop:
        cain    013,020
        jrst    dtfs_readdir_its
        move    6,4
        caie    013,010
        lsh     6,1                     ; native: two words/name

dtfs_readdir_tenex_base:
        addi    6,0123                  ; DTFS_NAME_BASE
        add     6,dtfs_dir
        skipn   (6)
        jrst    dtfs_readdir_next
        jrst    dtfs_readdir_present

dtfs_readdir_its:
        move    6,4
        lsh     6,1
        add     6,dtfs_dir
        skipe   (6)
        jrst    dtfs_readdir_present
        skipn   1(6)
        jrst    dtfs_readdir_next

dtfs_readdir_present:
        came    5,011
        jrst    dtfs_readdir_seen_next
        cain    013,020
        jrst    dtfs_readdir_name_its
        cain    013,010
        jrst    dtfs_readdir_name_tenex

        move    7,(6)
        movem   7,1(012)
        move    7,1(6)
        andcmi  7,077
        movem   7,2(012)
        setzm   3(012)
        setzm   4(012)
        movei   1,1(012)
        movei   2,013
        pushj   17,vfs_sixbit_name_chars
        movem   1,(012)
        jrst    dtfs_readdir_found

dtfs_readdir_name_tenex:
        move    1,4
        move    2,012
        setz    3,
        pushj   17,dtfs_foreign_name
        jrst    dtfs_readdir_found

dtfs_readdir_name_its:
        move    1,4
        move    2,012
        movei   3,1
        pushj   17,dtfs_foreign_name

dtfs_readdir_found:
        movei   1,2                     ; VFS_TYPE_REG
        movem   1,5(012)
        movei   1,1
        jrst    dtfs_readdir_return

dtfs_readdir_seen_next:
        addi    5,1

dtfs_readdir_next:
        addi    4,1
        cain    013,020
        jrst    dtfs_readdir_its_limit
        caige   4,026                   ; native/TENEX: 22 slots
        jrst    dtfs_readdir_loop
        jrst    dtfs_readdir_end
dtfs_readdir_its_limit:
        caige   4,027                   ; ITS: 23 slots
        jrst    dtfs_readdir_loop
dtfs_readdir_end:
        setz    1,                      ; end of directory
        jrst    dtfs_readdir_return

dtfs_readdir_fail:
        seto    1,
dtfs_readdir_return:
        jrst    dtfs_restore4

.else
; Native-only compact directory enumeration.
dtfs_readdir:
        push    17,010
        push    17,011
        push    17,012
        move    012,1                   ; dir survives dtfs_is_root
        move    010,2                   ; requested visible entry
        move    011,3                   ; result
        pushj   17,dtfs_is_root
        jumpe   1,dtfs_readdir_native_fail
        jumpe   011,dtfs_readdir_native_fail
        move    1,012
        pushj   17,dtfs_load
        jumpn   1,dtfs_readdir_native_fail
        setzb   4,5                     ; slot, seen
dtfs_readdir_native_loop:
        move    6,4
        lsh     6,1
        addi    6,0123
        add     6,dtfs_dir
        skipn   (6)
        jrst    dtfs_readdir_native_next
        came    5,010
        jrst    dtfs_readdir_native_seen
        move    7,(6)
        movem   7,1(011)
        move    7,1(6)
        andcmi  7,077
        movem   7,2(011)
        setzm   3(011)
        setzm   4(011)
        movei   1,1(011)
        movei   2,013
        pushj   17,vfs_sixbit_name_chars
        movem   1,(011)
        movei   1,2
        movem   1,5(011)
        movei   1,1
        jrst    dtfs_readdir_native_return
dtfs_readdir_native_seen:
        addi    5,1
dtfs_readdir_native_next:
        addi    4,1
        caige   4,026
        jrst    dtfs_readdir_native_loop
        setz    1,
        jrst    dtfs_readdir_native_return
dtfs_readdir_native_fail:
        seto    1,
dtfs_readdir_native_return:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

.endif

        .globl  dtfs_find_free_block
.if DTFS_ENABLE_FOREIGN
dtfs_find_free_block:
        push    17,010
        push    17,011
        move    010,3                   ; blockp
        jumpe   2,dtfs_find_free_native
        movei   6,1
        movei   011,1                   ; TENEX map is block-1
        jrst    dtfs_find_free_start

dtfs_find_free_native:
        move    6,1
        setz    011,
        caig    6,1
        jrst    dtfs_find_free_reset
        caig    6,01101
        jrst    dtfs_find_free_start

dtfs_find_free_reset:
        movei   6,0145                  ; first data block after directory

dtfs_find_free_start:
        setz    7,
dtfs_find_free_loop:
        setz    1,
        move    2,6
        sub     2,011
        pushj   17,dtfs_owner
        jumpe   1,dtfs_find_free_found
        addi    6,1
        caile   6,01101
        movei   6,1
dtfs_find_free_count:
        addi    7,1
        caige   7,01101
        jrst    dtfs_find_free_loop
        seto    1,
        jrst    dtfs_find_free_return

dtfs_find_free_found:
        movem   6,(010)
        setz    1,
dtfs_find_free_return:
        jrst    dtfs_restore2
.else
dtfs_find_free_block:
        push    17,010
        move    010,3                   ; blockp
        move    6,1                     ; preferred first block
        caig    6,1
        jrst    dtfs_find_free_native_reset
        caig    6,01101
        jrst    dtfs_find_free_native_start
dtfs_find_free_native_reset:
        movei   6,0145                  ; first native data block
dtfs_find_free_native_start:
        setz    7,
dtfs_find_free_native_loop:
        setz    1,
        move    2,6
        pushj   17,dtfs_owner
        jumpe   1,dtfs_find_free_native_found
        addi    6,1
        caile   6,01101
        movei   6,1
dtfs_find_free_native_count:
        addi    7,1
        caige   7,01101
        jrst    dtfs_find_free_native_loop
        pop     17,010
        jrst    kret_neg1
dtfs_find_free_native_found:
        movem   6,(010)
        pop     17,010
        jrst    kret_zero
.endif

        .globl  dtfs_block_info
        .globl  dtfs_size_words
.if DTFS_ENABLE_FOREIGN
dtfs_size_words:
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        pushj   17,dtfs_personality
        caie    1,010
        jrst    dtfs_size_words_native
        move    1,010
        pushj   17,dtfs_unit
        move    2,011
        setzb   3,4
        add     17,kconst_2_2
        setzm   (17)                    ; fifth argument nwords = 0
        movei   5,1
        movem   5,-1(17)                ; sixth argument map offset = 1
        pushj   17,dtfs_chain_walk
        sub     17,kconst_2_2
        jumpl   1,dtfs_size_words_zero
        jrst    dtfs_size_words_return

dtfs_size_words_native:
        move    1,010
        move    2,011
        setz    3,
        pushj   17,dtfs_block_info
        jumpe   1,dtfs_size_words_zero
        move    4,011
        lsh     4,1
        add     4,dtfs_dir
        move    2,0124(4)
        andi    2,077
        move    3,011
        add     3,dtfs_dir
        move    3,026(3)
        trne    3,1
        iori    2,0100
        cail    2,1
        cail    2,0200
        jrst    dtfs_size_words_zero
        move    3,1
        lsh     1,7
        sub     1,3
        add     1,2
        subi    1,0177
        jrst    dtfs_size_words_return

dtfs_size_words_zero:
        setz    1,
dtfs_size_words_return:
        jrst    dtfs_restore2
.else
dtfs_size_words:
        push    17,010
        move    010,2                   ; slot
        setz    3,
        pushj   17,dtfs_block_info
        jumpe   1,dtfs_size_words_native_zero
        move    4,010
        lsh     4,1
        add     4,dtfs_dir
        move    2,0124(4)
        andi    2,077
        move    3,010
        add     3,dtfs_dir
        move    3,026(3)
        trne    3,1
        iori    2,0100
        cail    2,1
        cail    2,0200
        jrst    dtfs_size_words_native_zero
        move    3,1
        lsh     1,7
        sub     1,3
        add     1,2
        subi    1,0177
dtfs_size_words_native_return:
        pop     17,010
        popj    17,
dtfs_size_words_native_zero:
        setz    1,
        jrst    dtfs_size_words_native_return
.endif

        .globl  dtfs_set_name
dtfs_set_name:
        lsh     1,1
        add     1,dtfs_dir
        move    3,0124(1)
        andi    3,077
        move    4,1(2)
        movem   4,0123(1)
        move    4,2(2)
        dpb     3,[POINT 6,4,35]
        movem   4,0124(1)
        popj    17,
