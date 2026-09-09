; dtfs_pdp10.s -- DTFS runtime and boot-patched DTC veneers.
        .text

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
        .globl  dtfs_format_unit
        .globl  dtfs_mount_unit

dtfs_mres_dispatch:
        movei   7,5
        jrst    fs_provider_request_call
dtfs_mres_reg_dispatch:
        move    7,[dtfs_mres_vector]
        jrst    fs_mres_vector_dispatch

        .data
dtfs_mres_vector:
        .word   017                      ; highest runtime VFS operation: 15
        movei   7,dtfs_lookup                 ; 1
        movei   7,dtfs_readdir                ; 2
        movei   7,dtfs_stat                   ; 3
        movei   7,dtfs_parent                 ; 4
        jrst    fs_mres_no_service       ; 5 PARENT_NAME unsupported
        movei   7,dtfs_create                 ; 6
        movei   7,dtfs_mkdir_unsupported      ; 7 MKDIR unsupported
        jrst    fs_mres_no_service       ; 8 SYMLINK unsupported
        movei   7,dtfs_unlink                 ; 9 UNLINK
        movei   7,dtfs_rename                 ; 10 RENAME
        movei   7,dtfs_truncate               ; 11 TRUNCATE
        movei   7,dtfs_chmod                  ; 12 CHMOD
        movei   7,dtfs_read_words             ; 13 READ_WORDS
        movei   7,dtfs_write_words            ; 14 WRITE_WORDS
        movei   7,dtfs_sync                   ; 15 SYNC
        .text

; DTFS is deliberately flat.  Preserve a distinct error through the syscall
; boundary so usr can report that MKDIR is unsupported rather than an
; undifferentiated filesystem failure.
dtfs_mkdir_unsupported:
        jrst    pdp10_ret_neg2  ; SYS_ERR_UNSUPPORTED (-2)


; Compact five-bit allocation-map accessors.  All DTFS map indices are small
; non-negative values, so IDIVI avoids GCC's 72-bit unsigned DIV setup.
        .globl  dtfs_owner
dtfs_owner:
        idivi   2,7                     ; AC2 word index, AC3 remainder
        add     1,2                     ; AC1 map base + word index
        move    2,3
        lsh     2,2
        add     2,3                     ; remainder * 5
        move    1,dtfs_dir(1)
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
        andca   3,dtfs_dir(1)
        lsh     4,0(5)
        ior     3,4
        movem   3,dtfs_dir(1)
        popj    17,

.if DTFS_ENABLE_TENEX
; TENEX directory validation, including the optional fsck/deep chain pass.
; The allocation index and slot ranges are small non-negative constants, so
; direct CAIGE loops avoid GCC's signed-range scaffolding.
        .globl  dtfs_chain_walk
        .globl  dtfs_tenex_valid
dtfs_tenex_valid:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; unit for optional deep walk
        move    011,2                   ; deep flag

; TENEX DECTAP.MAC DTINID/DIRTHR structural markers.
        move    4,dtfs_dir
        lsh     4,-032
        caie    4,01736
        jrst    dtfs_tenex_valid_false
        ldb     4,[POINT 5,dtfs_dir+016,9]
        caie    4,036
        jrst    dtfs_tenex_valid_false
        move    4,dtfs_dir+0122
        and     4,[07777776]
        came    4,[07777776]
        jrst    dtfs_tenex_valid_false

; Validate all 578 allocation entries and remember which file owners occur.
        setz    012,                    ; seen owner bitmap
        setz    013,                    ; allocation index
dtfs_tenex_valid_map_loop:
        setz    1,
        move    2,013
        pushj   17,dtfs_owner
        cail    1,027
        jrst    dtfs_tenex_valid_special_owner
        jumpe   1,dtfs_tenex_valid_map_next
        skipn   dtfs_dir+0122(1)        ; NAME_BASE + owner - 1
        jrst    dtfs_tenex_valid_false
        movei   4,1
        lsh     4,-1(1)                 ; bit owner-1
        ior     012,4
        jrst    dtfs_tenex_valid_map_next
dtfs_tenex_valid_special_owner:
        cail    1,036
        cail    1,040
        jrst    dtfs_tenex_valid_false

dtfs_tenex_valid_map_next:
        addi    013,1
        caige   013,01102
        jrst    dtfs_tenex_valid_map_loop

; Every named slot must own at least one block; an empty NAME may not carry an
; EXT.  Deep CHECK additionally validates the complete block chain in-place.
        setz    013,
dtfs_tenex_valid_slot_loop:
        skipn   dtfs_dir+0123(013)
        jrst    dtfs_tenex_valid_empty_slot
        move    4,012
        movn    5,013
        lsh     4,0(5)
        trnn    4,1
        jrst    dtfs_tenex_valid_false
        jumpe   011,dtfs_tenex_valid_slot_next
        move    1,010
        move    2,013
        setz    3,
        setz    4,
        add     17,[2,,2]
        setzm   (17)                    ; fifth argument nwords = 0
        movei   5,1
        movem   5,-1(17)                ; sixth argument map offset = 1
        pushj   17,dtfs_chain_walk
        sub     17,[2,,2]
        jumpl   1,dtfs_tenex_valid_false
        jrst    dtfs_tenex_valid_slot_next
dtfs_tenex_valid_empty_slot:
        skipn   dtfs_dir+0151(013)
        jrst    dtfs_tenex_valid_slot_next
        jrst    dtfs_tenex_valid_false

dtfs_tenex_valid_slot_next:
        addi    013,1
        caige   013,026
        jrst    dtfs_tenex_valid_slot_loop
        movei   1,1
        jrst    dtfs_tenex_valid_return
dtfs_tenex_valid_false:
        setz    1,
dtfs_tenex_valid_return:
.endif

dtfs_restore4:
        pop     17,013
dtfs_restore3:
        pop     17,012
dtfs_restore2:
        pop     17,011
dtfs_restore1:
        pop     17,010
        popj    17,

.if DTFS_ENABLE_ITS
; ITS directory structural validation.  Keep the fixed UTAPE markers and
; owner/name consistency scan in one compact target loop.
        .globl  dtfs_its_valid
dtfs_its_valid:
        move    1,dtfs_dir+056
        andcm   1,[1]
        came    1,[0757367573674]
        jrst    dtfs_its_valid_false
        move    1,dtfs_dir+067
        lsh     1,-037                  ; owner is the top five bits
        caie    1,033
        jrst    dtfs_its_valid_false
        move    1,dtfs_dir+0177
        andcm   1,[1]
        came    1,[0777777777776]
        jrst    dtfs_its_valid_false
        push    17,010
        movei   010,7

dtfs_its_valid_loop:
        movei   1,056
        move    2,010
        pushj   17,dtfs_owner
        caie    1,037
        jrst    dtfs_its_valid_owner
        jrst    dtfs_its_valid_pop_false
dtfs_its_valid_owner:
        jumpe   1,dtfs_its_valid_next
        caile   1,027
        jrst    dtfs_its_valid_next
        subi    1,1
        lsh     1,1
        skipn   dtfs_dir(1)
        skipe   dtfs_dir+1(1)
        jrst    dtfs_its_valid_next
        jrst    dtfs_its_valid_pop_false

dtfs_its_valid_next:
        addi    010,1
        caige   010,01067
        jrst    dtfs_its_valid_loop
        pop     17,010
        jrst    pdp10_ret_one
dtfs_its_valid_pop_false:
        pop     17,010
dtfs_its_valid_false:
        jrst    pdp10_ret_zero

; Compact vnode predicates.  The vnode encoding is provider:6, kind/mount:12,
; index:18.  Mask provider plus local kind in one operation; mount-id and file
; index remain independent tests.
.endif

        .globl  dtfs_is_root
dtfs_is_root:
        move    2,1
        and     2,[770077000000]
        came    2,[050001000000]       ; DTFS provider, root local kind
        jrst    pdp10_ret_zero
        move    2,1
        and     2,[007700000000]       ; non-zero mount id required
        jumpe   2,pdp10_ret_zero
        jrst    pdp10_ret_one

        .globl  dtfs_is_file
dtfs_is_file:
        move    2,1
        and     2,[770077000000]
        came    2,[050002000000]       ; DTFS provider, file local kind
        jrst    pdp10_ret_zero
        move    2,1
        and     2,[007700000000]
        jumpe   2,pdp10_ret_zero
        move    2,1
        andi    2,0777777
        cail    2,027                  ; ITS has 23 file slots
        jrst    pdp10_ret_zero
        jrst    pdp10_ret_one

; Flat DTFS parent and sync operations need only vnode classification.  The
; local predicates touch AC1/AC2 only, so AC4/AC5 can retain the original
; arguments without a compiler-generated stack frame.
        .globl  dtfs_parent
dtfs_parent:
        move    4,1                     ; original file vnode
        move    5,2                     ; vnode_t *parentp
        jumpe   5,pdp10_ret_neg1
        pushj   17,dtfs_is_file
        jumpe   1,pdp10_ret_neg1
        and     4,[07700000000]         ; retain mount id
        tlo     4,050001                 ; DTFS provider + root local kind
        movem   4,(5)
        jrst    pdp10_ret_zero
        .globl  dtfs_sync
dtfs_sync:
        move    4,1
        pushj   17,dtfs_is_root
        jumpn   1,dtfs_sync_ok
        move    1,4
        pushj   17,dtfs_is_file
        jumpe   1,pdp10_ret_neg1
dtfs_sync_ok:
        jrst    pdp10_ret_zero
; DTC veneers.  MINIT patches the RH of each JRST with the installed DTC
; service entry.  The DTFS and DTC ABIs are identical: AC1=unit, AC2=block,
; AC3=buffer, so the tail jump needs no argument shuffling or resident pointer.
        .globl  dtfs_dtc_read
        .globl  dtfs_dtc_read_jump
dtfs_dtc_read:
dtfs_dtc_read_jump:
        jrst    0

        .globl  dtfs_dtc_write
        .globl  dtfs_dtc_write_jump
dtfs_dtc_write:
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
        move    5,dtfs_dir(1)
        move    6,dtfs_dir+1(1)
        movei   7,6                     ; ITS extension limit
        jrst    dtfs_foreign_name_have_words
dtfs_foreign_name_tenex:
        move    5,dtfs_dir+0123(1)
        move    6,dtfs_dir+0151(1)
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
        move    010,1
        lsh     010,1
        addi    010,dtfs_dir
        tlo     010,1                   ; ITS layout tag in LH
        movei   7,6                     ; maximum EXT length
        jrst    dtfs_foreign_set_name_setup_done
dtfs_foreign_set_name_tenex_setup:
        movei   010,dtfs_dir+0123(1)
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
        movei   0,6
        sub     0,1
        imuli   0,6
        lsh     5,0(0)
        movei   0,6
        sub     0,2
        imuli   0,6
        lsh     6,0(0)
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
        jumple  4,pdp10_ret_neg1
        caile   4,013                    ; DTFS_NAME_MAX_CHARS = 11
        jrst    pdp10_ret_neg1
        skipe   3(1)
        jrst    pdp10_ret_neg1
        skipe   4(1)
        jrst    pdp10_ret_neg1
dtfs_native_scan_begin:
        setz    3,                       ; slot
        movei   4,0123                   ; DTFS_NAME_BASE
dtfs_native_scan_loop:
        jumpe   1,dtfs_native_scan_empty
        move    5,dtfs_dir(4)
        came    5,1(1)
        jrst    dtfs_native_scan_next
        move    5,dtfs_dir+1(4)
        andcmi  5,077                    ; ignore native tail-count bits
        move    6,2(1)
        andcmi  6,077
        came    5,6
        jrst    dtfs_native_scan_next
        jrst    dtfs_native_scan_match
dtfs_native_scan_empty:
        skipn   dtfs_dir(4)
        jrst    dtfs_native_scan_match
dtfs_native_scan_next:
        addi    4,2
        addi    3,1
        caige   3,026                    ; DTFS_FILE_SLOTS = 22
        jrst    dtfs_native_scan_loop
        jrst    pdp10_ret_neg1
dtfs_native_scan_match:
        jumpe   2,pdp10_ret_zero
        movem   3,(2)
        jrst    pdp10_ret_zero

        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

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
        and     010,[07700000000]       ; retain mount id
        tlo     010,050002              ; DTFS provider + file local kind
        hrr     010,(17)                ; slot index
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
dtfs_chmod:
        push    17,010
        push    17,011
        move    010,1
        move    011,2
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
        jrst    pdp10_ret_neg1

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
        popj    17,

        .globl  dtfs_personality
dtfs_personality:
        ldb     2,[POINT 6,1,11]
        xct     dtfs_personality_xct-1(2)
        popj    17,

; Shared cached directory loader.  Keep only mount id and packed media across
; the DTC/validator calls; unit and personality are cheap masks of MEDIA.
        .globl  dtfs_cache_mount
        .globl  dtfs_media
        .globl  dtfs_native_valid

dtfs_load:
        push    17,010
        push    17,011
        ldb     010,[POINT 6,1,11]      ; mount id
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_ok
        move    011,dtfs_media-1(010)   ; unit + personality
        move    1,011
        andi    1,7                     ; unit
        xct     dtfs_personality_xct-1(010)
        move    4,1                     ; personality
        movei   2,0144                  ; native/TENEX directory
        cain    4,020
        movei   2,0100                  ; ITS directory
        movei   3,dtfs_dir
        pushj   17,dtfs_dtc_read
        jumpn   1,dtfs_load_fail
        xct     dtfs_personality_xct-1(010)
        move    4,1
        cain    4,020
        jrst    dtfs_load_validate_its
        cain    4,010
        jrst    dtfs_load_validate_tenex
        pushj   17,dtfs_native_valid
        jrst    dtfs_load_validated

dtfs_load_validate_its:
        pushj   17,dtfs_its_valid
        jrst    dtfs_load_validated

dtfs_load_validate_tenex:
        move    1,011
        andi    1,7
        setz    2,
        pushj   17,dtfs_tenex_valid

dtfs_load_validated:
        jumpe   1,dtfs_load_fail
        movem   010,dtfs_cache_mount

dtfs_load_ok:
        setz    1,
        jrst    dtfs_load_return

dtfs_load_fail:
        seto    1,
dtfs_load_return:
        jrst    dtfs_restore2

.else
; Native-only build: packed media contains only the unit.
        .globl  dtfs_patch_media
dtfs_patch_media:
        andi    2,7
        movem   2,dtfs_media-1(1)
        popj    17,

        .globl  dtfs_cache_mount
        .globl  dtfs_media
        .globl  dtfs_native_valid

dtfs_load:
        push    17,010
        ldb     010,[POINT 6,1,11]
        move    4,dtfs_cache_mount
        camn    4,010
        jrst    dtfs_load_native_ok
        move    1,dtfs_media-1(010)
        andi    1,7
        movei   2,0144
        movei   3,dtfs_dir
        pushj   17,dtfs_dtc_read
        jumpn   1,dtfs_load_native_fail
        pushj   17,dtfs_native_valid
        jumpe   1,dtfs_load_native_fail
        movem   010,dtfs_cache_mount
dtfs_load_native_ok:
        pop     17,010
        jrst    pdp10_ret_zero
dtfs_load_native_fail:
        pop     17,010
        jrst    pdp10_ret_neg1

.endif

; Native directory validation.  The three post-media map entries are a fixed
; tiny range, so use a direct CAIG loop instead of GCC's signed-range code.
dtfs_native_valid:
        move    1,dtfs_dir+0177
        came    1,[0446446632021]
        jrst    pdp10_ret_zero
        setzb   1,2
        pushj   17,dtfs_owner
        caie    1,036
        jrst    pdp10_ret_zero
        setz    1,
        movei   2,0144
        pushj   17,dtfs_owner
        caie    1,036
        jrst    pdp10_ret_zero
        movei   4,01102

dtfs_native_valid_loop:
        setz    1,
        move    2,4
        pushj   17,dtfs_owner
        caie    1,035
        jrst    pdp10_ret_zero
        addi    4,1
        caig    4,01104
        jrst    dtfs_native_valid_loop
        jrst    pdp10_ret_one


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
        cain    013,010
        jrst    dtfs_readdir_tenex_base
        lsh     6,1                     ; native: two words/name

dtfs_readdir_tenex_base:
        addi    6,0123                  ; DTFS_NAME_BASE
        skipn   dtfs_dir(6)
        jrst    dtfs_readdir_next
        jrst    dtfs_readdir_present

dtfs_readdir_its:
        move    6,4
        lsh     6,1
        skipe   dtfs_dir(6)
        jrst    dtfs_readdir_present
        skipn   dtfs_dir+1(6)
        jrst    dtfs_readdir_next

dtfs_readdir_present:
        came    5,011
        jrst    dtfs_readdir_seen_next
        cain    013,020
        jrst    dtfs_readdir_name_its
        cain    013,010
        jrst    dtfs_readdir_name_tenex

        move    7,dtfs_dir(6)
        movem   7,1(012)
        move    7,dtfs_dir+1(6)
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
        move    010,2                   ; requested visible entry
        move    011,3                   ; result
        pushj   17,dtfs_is_root
        jumpe   1,dtfs_readdir_native_fail
        jumpe   011,dtfs_readdir_native_fail
        pushj   17,dtfs_load
        jumpn   1,dtfs_readdir_native_fail
        setzb   4,5                     ; slot, seen
dtfs_readdir_native_loop:
        move    6,4
        lsh     6,1
        addi    6,0123
        skipn   dtfs_dir(6)
        jrst    dtfs_readdir_native_next
        came    5,010
        jrst    dtfs_readdir_native_seen
        move    7,dtfs_dir(6)
        movem   7,1(011)
        move    7,dtfs_dir+1(6)
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
        caig    6,01101
        jrst    dtfs_find_free_count
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
        caig    6,01101
        jrst    dtfs_find_free_native_count
        movei   6,1
dtfs_find_free_native_count:
        addi    7,1
        caige   7,01101
        jrst    dtfs_find_free_native_loop
        pop     17,010
        jrst    pdp10_ret_neg1
dtfs_find_free_native_found:
        movem   6,(010)
        pop     17,010
        jrst    pdp10_ret_zero
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
        move    2,dtfs_dir+0124(4)
        andi    2,077
        move    3,dtfs_dir+026(011)
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
        move    2,dtfs_dir+0124(4)
        andi    2,077
        move    3,dtfs_dir+026(010)
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
        move    3,dtfs_dir+0124(1)
        andi    3,077
        move    4,1(2)
        movem   4,dtfs_dir+0123(1)
        move    4,2(2)
        dpb     3,[POINT 6,4,35]
        movem   4,dtfs_dir+0124(1)
        popj    17,
