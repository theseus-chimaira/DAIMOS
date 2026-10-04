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

dtfs_mres_dispatch:
dtfs_mres_reg_dispatch:
        cain    6,025                   ; FS_MRES_OP_SPACE
        jrst    dtfs_space
        move    7,[dtfs_mres_vector]
        jrst    fs_mres_vector_dispatch

; Return total/used physical data words for the mounted DECtape.  Reserved
; directory/format blocks are already marked non-free in every supported
; allocation map, so the same census covers native, TENEX and ITS media.
dtfs_space:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        ldb     4,[POINT 6,1,11]        ; public mount id
        sojl    4,dtfs_space_fail
        caile   4,3
        jrst    dtfs_space_fail
        move    5,dtfs_media(4)
        hrrz    5,5
        andi    5,030                   ; personality
        setz    010,                    ; map-base offset
        setz    013,                    ; block-number -> map-index offset
        cain    5,020                   ; ITS
        movei   010,056                 ; DTFS_ITS_NAME_WORDS
        jumpe   5,dtfs_space_scan       ; native uses block number directly
        movei   013,1                   ; TENEX/ITS use block-1
dtfs_space_scan:
        movei   011,1                   ; physical block 1..01101
        setz    012,                    ; used blocks
dtfs_space_loop:
        move    1,010
        move    2,011
        sub     2,013
        pushj   17,dtfs_owner
        jumpe   1,dtfs_space_next
        aoj     012,
dtfs_space_next:
        aoj     011,
        caile   011,01101
        jrst    dtfs_space_done
        jrst    dtfs_space_loop
dtfs_space_done:
        movei   1,01101
        lsh     1,7                     ; 128 words/block
        move    2,012
        lsh     2,7
        jrst    dtfs_restore4
dtfs_space_fail:
        seto    1,
        jrst    dtfs_restore4

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
        jrst    kret_zero
; Shared clean DTC block-cache veneers.  The key identifies the physical
; UNIT/BLOCK, not a filesystem personality, so native DTFS, TENEX, ITS, and
; TSFS can reuse the same clean copy.  MINIT patches the raw JRST slots below.
        .globl  dtfs_dtc_read
        .globl  bcache_fetch
        .globl  bcache_store
        .globl  bcache_reclaim
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
        jumpn   1,dtfs_dtc_read_done
        move    1,-1(17)                ; successful miss becomes clean hit
        move    2,(17)
        pushj   17,bcache_store
dtfs_dtc_read_hit:
        setz    1,
dtfs_dtc_read_done:
        sub     17,[2,,2]
        popj    17,

        .globl  dtfs_dtc_read_jump
dtfs_dtc_read_jump:
        jrst    0

        .globl  dtfs_dtc_write
dtfs_dtc_write:
        pushj   17,bcache_reclaim        ; invalidate before authoritative write
        jrst    dtfs_dtc_write_jump

        .globl  dtfs_dtc_write_jump
dtfs_dtc_write_jump:
        jrst    0

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
        add     17,[5,,5]              ; temporary struct vfs_name
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
        sub     17,[5,,5]
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
        sub     17,[2,,2]
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
        setzm   dtfs_cache_mount
        move    1,(17)
        movem   1,(013)
        setz    1,
        jrst    dtfs_mount_unit_done_local

dtfs_mount_unit_fail_local:
        seto    1,
dtfs_mount_unit_done_local:
        sub     17,[1,,1]
        jrst    dtfs_restore4
dtfs_mount_unit_fail:
        seto    1,
        jrst    dtfs_restore4

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
        sub     17,[1,,1]
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
; Per-mount personality instructions are patched once by mount.  XCT turns a
; personality lookup into MOVEI 1,{0,010,020}; dtfs_media[] remains the packed
; unit/personality store used by paths that need the unit too.
        .data
dtfs_personality_xct:
        movei   1,0
        movei   1,0
        movei   1,0
        movei   1,0
        .text

        .globl  dtfs_patch_media
dtfs_patch_media:
        movem   2,dtfs_media-1(1)
        move    3,dtfs_personality_xct  ; MOVEI 1,0 template
        andi    2,030
        ior     3,2
        movem   3,dtfs_personality_xct-1(1)
        pushj   17,bcache_reclaim       ; source id may refer to new media
        popj    17,

        .globl  dtfs_personality
dtfs_personality:
        ldb     2,[POINT 6,1,11]
        xct     dtfs_personality_xct-1(2)
        popj    17,

 ; Mount ownership is VFS policy, never DECtape media metadata.  The high
; half of each dtfs_media word carries compact UID9,GID9 while its low half
; remains the existing unit/personality value.

; void dtfs_stat_owner(vnode, struct vfs_stat *st)
; dtfs_media LH is compact UID9,,GID9 for the mount.
        .globl  dtfs_stat_owner
dtfs_stat_owner:
        ldb     3,[POINT 6,1,11]        ; mount id 1..4
        subi    3,1
        hlrz    4,dtfs_media(3)         ; owner18
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
        .globl  dtfs_media

dtfs_load:
        push    17,010
        ldb     010,[POINT 6,1,11]      ; mount id
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_ok
        move    4,dtfs_media-1(010)     ; unit + personality
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
; Native-only build: packed media contains only the unit.
        .globl  dtfs_patch_media
dtfs_patch_media:
        movem   2,dtfs_media-1(1)
        pushj   17,bcache_reclaim
        popj    17,

        .globl  dtfs_cache_mount
        .globl  dtfs_media

dtfs_load:
        push    17,010
        ldb     010,[POINT 6,1,11]
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_native_ok
        move    1,dtfs_media-1(010)
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
        add     17,[2,,2]
        setzm   (17)                    ; fifth argument nwords = 0
        movei   5,1
        movem   5,-1(17)                ; sixth argument map offset = 1
        pushj   17,dtfs_chain_walk
        sub     17,[2,,2]
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
