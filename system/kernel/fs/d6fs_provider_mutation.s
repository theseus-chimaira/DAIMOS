        .text
        .globl  d6fs_provider_write_dirent
        .globl  pclk_time36
; int d6fs_provider_write_dirent(dir, slot, di)
;
; The directory FCB has already passed d6fs_provider_fcb validation.  D6FS
; geometry keeps both the directory offset and size below the signed 36-bit
; range, so direct CAMG comparison is equivalent to the C unsigned compare.
; AC010 keeps DIR, AC011 becomes the six-word directory offset, and AC012 keeps
; DI until the raw entry has been formed.
d6fs_provider_write_dirent:
        add     17,[044,,044]            ; saved ACs + FCB/info/raw + arg 5
        movem   010,-043(17)
        movem   011,-042(17)
        movem   012,-041(17)
        move    010,1
        move    011,2
        move    012,3
        movei   2,-040(17)               ; 020-word FCB
        movei   3,-020(17)               ; 012-word FCB info
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_write_dirent_fail
        move    4,-020(17)               ; fi.type
        caie    4,2                      ; D6FS_TYPE_DIR
        jrst    d6fs_provider_write_dirent_fail
        move    4,-017(17)               ; fi.flags
        trne    4,020                    ; D6FS_FLAG_IMMUTABLE
        jrst    d6fs_provider_write_dirent_fail
        jumpe   012,d6fs_provider_write_dirent_zero
        move    1,012
        movei   2,-6(17)
        movei   3,4
        pushj   17,fs_copy_words
        move    3,4(012)
        lsh     3,014
        move    4,5(012)
        lsh     4,011
        ior     3,4
        ior     3,6(012)
        movem   3,-2(17)
        move    3,7(012)
        hrlzm   3,-1(17)
        jrst    d6fs_provider_write_dirent_raw_done

d6fs_provider_write_dirent_zero:
        movei   1,-6(17)
        movei   2,6
        pushj   17,fs_zero_words

d6fs_provider_write_dirent_raw_done:
        imuli   011,6                    ; off = slot * D6FS_DIRENT_WORDS
        move    4,011
        addi    4,6                      ; need
        camg    4,-011(17)               ; fi.size_words
        jrst    d6fs_provider_write_dirent_store
        move    1,010
        movei   2,-040(17)
        movei   3,-020(17)
        pushj   17,d6fs_provider_resize_fcb
        jumpn   1,d6fs_provider_write_dirent_fail

d6fs_provider_write_dirent_store:
        movei   5,6
        movem   5,(17)                   ; fifth argument: nwords
        move    1,d6fs_active_reader
        movei   2,-040(17)
        move    3,011
        movei   4,-6(17)
        pushj   17,d6fs_reader_write_words
        caie    1,6
        jrst    d6fs_provider_write_dirent_fail
        pushj   17,pclk_time36
        movem   1,-035(17)               ; directory FCB MTIME: -040 + 3
        move    1,d6fs_active_reader
        hrrz    2,010                    ; directory FCB index
        movei   3,-040(17)
        pushj   17,d6fs_reader_put_fcb
        jumpn   1,d6fs_provider_write_dirent_fail
        setz    1,
        jrst    d6fs_provider_write_dirent_done

d6fs_provider_write_dirent_fail:
        seto    1,
d6fs_provider_write_dirent_done:
        move    010,-043(17)
        move    011,-042(17)
        move    012,-041(17)
        sub     17,[044,,044]
        popj    17,

        .globl  d6fs_provider_free_fcb
; int d6fs_provider_free_fcb(indexp)
;
; SUPER.FCB_COUNT is validated and small (currently at most the fixed FCB
; table capacity), so a direct machine-word comparison is sufficient.  The scratch
; frame is exactly one FCB plus one decoded-info object; only AC010/AC011 live
; across d6fs_reader_fcb calls.
d6fs_provider_free_fcb:
        add     17,[034,,034]
        movem   010,-033(17)
        movem   011,-032(17)
        move    010,1                    ; indexp
        movei   011,1                    ; FCB zero is reserved

d6fs_provider_free_fcb_loop:
        move    5,d6fs_active_reader
        jumpe   5,d6fs_provider_free_fcb_fail
        caml    011,011(5)
        jrst    d6fs_provider_free_fcb_fail
        move    1,d6fs_active_reader
        move    2,011
        movei   3,-031(17)               ; 020-word FCB scratch
        movei   4,-011(17)               ; 012-word decoded info
        pushj   17,d6fs_reader_fcb
        jumpn   1,d6fs_provider_free_fcb_next
        skipe   -011(17)                 ; fi.type == D6FS_TYPE_FREE
        jrst    d6fs_provider_free_fcb_next
        movem   011,(010)
        setz    1,
        jrst    d6fs_provider_free_fcb_done

d6fs_provider_free_fcb_next:
        aoja    011,d6fs_provider_free_fcb_loop

d6fs_provider_free_fcb_fail:
        seto    1,
d6fs_provider_free_fcb_done:
        move    010,-033(17)
        move    011,-032(17)
        sub     17,[034,,034]
        popj    17,
        .globl  d6fs_provider_resize_fcb
; int d6fs_provider_resize_fcb(node, fcb, fi, new_words)
;
; PDP-6/PDP-10 implementation of the complete resize transaction.  Keeping the
; allocation engine and publication/rollback policy in one routine avoids the
; large KCC stack/call scaffolding that otherwise dominates this hot path.
;
; Frame (036 words):
;   -033..-025 saved AC010..AC016
;   -024..-010 saved old FCB (015 words)
;   -007 node, -006 fcb, -005 fi, -004 new_words, -003 reserved
;   -002 old_blocks, -001 new_blocks, 0 new extent_count
;   -035 returned start, -034 returned blocks (growth scratch)
d6fs_provider_resize_fcb:
        add     17,[036,,036]
        movem   010,-033(17)
        movem   011,-032(17)
        movem   012,-031(17)
        movem   013,-030(17)
        movem   014,-027(17)
        movem   015,-026(17)
        movem   016,-025(17)
        movem   1,-7(17)
        movem   2,-6(17)
        movem   3,-5(17)
        movem   4,-4(17)
        move    1,d6fs_active_reader
        jumpe   1,d6fs_resize_fail
        move    2,1(1)
        trnn    2,0100                    ; D6FS_PROVIDER_MOUNT_WRITABLE
        jrst    d6fs_resize_fail

        ; Preserve the on-disk portion for rollback (words 0..014).
        movei   1,-024(17)
        hrl     1,-6(17)
        blt     1,-010(17)
        move    1,-5(17)
        move    2,4(1)                    ; fi.extent_count
        movem   2,(17)
        move    2,7(1)                    ; fi.size_words
        addi    2,0177
        lsh     2,-7
        movem   2,-2(17)                  ; old_blocks
        move    3,-4(17)
        addi    3,0177
        lsh     3,-7
        movem   3,-1(17)                  ; new_blocks
        move    1,d6fs_active_reader
        camg    3,6(1)                    ; reject new_blocks > total_blocks
        jrst    d6fs_resize_blocks_ok
        jrst    d6fs_resize_fail

d6fs_resize_blocks_ok:
        ; The extent engine uses AC010..AC016 as persistent state.
        move    010,-6(17)                ; fcb
        movei   011,-024(17)              ; old_fcb
        move    012,-2(17)                ; old_blocks
        move    013,-1(17)                ; new_blocks
        move    014,(17)                  ; extent_count
        move    015,013
        sub     015,012                    ; signed block-count delta
        jumpg   015,d6fs_resize_grow
        jumpl   015,d6fs_resize_shrink
        jrst    d6fs_resize_publish

; Grow the final extent in place while the following blocks are free.  Each
; newly allocated adjacent block is reflected in the FCB immediately so the
; common rollback can discover and release it if a later operation fails.
d6fs_resize_grow:
        jumpe   014,d6fs_resize_new_run
        move    016,014
        subi    016,1
        move    1,5(010)
        move    2,016
        lsh     2,2
        add     2,016                    ; extent index * 5
        movn    2,2
        lsh     1,0(2)
        andi    1,037
        move    3,010
        pushj   17,d6fs_resize_decode_extent

; Decode one packed FCB extent during resize.  This path is used only by
; truncate/grow metadata work, so one PUSHJ saves the duplicated nine-word
; unpack sequence without adding overhead to normal file I/O.
; AC1 = packed high-start bits, AC3 = FCB base, AC16 = extent index.
; Returns AC12 = start block, AC13 = block count.
d6fs_resize_decode_extent:
        addi    3,6
        add     3,016
        move    2,(3)
        ldb     012,[POINT 24,2,23]
        andi    2,07777
        lsh     1,014
        ior     2,1
        addi    2,1
        move    013,2
        popj    17,

d6fs_resize_extend_loop:
        jumpe   015,d6fs_resize_publish
        caml    013,[0200000]
        jrst    d6fs_resize_new_run
        move    2,012
        add     2,013                    ; adjacent candidate
        move    1,d6fs_active_reader
        caml    2,6(1)
        jrst    d6fs_resize_new_run
        pushj   17,d6fs_freemap_state
        jumpn   1,d6fs_resize_new_run
        move    2,012
        add     2,013
        move    1,d6fs_active_reader
        pushj   17,d6fs_reader_zero_block
        jumpn   1,d6fs_resize_rollback
        move    2,012
        add     2,013
        move    1,d6fs_active_reader
        movei   3,1
        pushj   17,d6fs_freemap_set
        jumpn   1,d6fs_resize_rollback
        aoj     013,
        soj     015,
        move    1,010
        move    2,016
        move    3,012
        move    4,013
        pushj   17,d6fs_provider_set_extent
        jrst    d6fs_resize_extend_loop

; Allocate one new contiguous run.  The two top frame words are scratch output
; cells for d6fs_provider_alloc_run(): -1=start, 0=blocks.  OLD/NEW block
; counts are no longer needed once growth has been selected.
d6fs_resize_new_run:
        jumpe   015,d6fs_resize_publish
        cail    014,7
        jrst    d6fs_resize_rollback
        move    1,015
        caile   1,0200000
        movei   1,0200000
        movem   1,-034(17)
        movei   2,-035(17)
        movei   3,-034(17)
        pushj   17,d6fs_provider_alloc_run
        jumpn   1,d6fs_resize_rollback
        setz    016,                     ; committed blocks in this run

d6fs_resize_zero_run:
        move    1,016
        caml    1,-034(17)
        jrst    d6fs_resize_run_ready
        move    2,-035(17)
        add     2,016
        move    1,d6fs_active_reader
        pushj   17,d6fs_reader_zero_block
        jumpn   1,d6fs_resize_run_partial_fail
        move    2,-035(17)
        add     2,016
        move    1,d6fs_active_reader
        movei   3,1
        pushj   17,d6fs_freemap_set
        jumpn   1,d6fs_resize_run_partial_fail
        aoja    016,d6fs_resize_zero_run

d6fs_resize_run_partial_fail:
        jumpe   016,d6fs_resize_rollback
        move    1,d6fs_active_reader
        move    2,-035(17)
        move    3,016
        pushj   17,d6fs_free_run
        jrst    d6fs_resize_rollback

d6fs_resize_run_ready:
        move    1,010
        move    2,014
        move    3,-035(17)
        move    4,-034(17)
        pushj   17,d6fs_provider_set_extent
        aoj     014,
        move    1,-034(17)
        sub     015,1
        move    2,-035(17)
        add     2,1
        move    1,d6fs_active_reader
        movem   2,(1)
        caml    2,6(1)
        setzm   (1)
        jrst    d6fs_resize_new_run

; Shrink reconstructs only the retained prefix.  Media is not released until
; the smaller FCB has been written successfully in d6fs_resize_publish.
d6fs_resize_shrink:
        move    015,013                  ; blocks to keep
        setz    016,                     ; retained extent index

d6fs_resize_shrink_loop:
        jumpe   015,d6fs_resize_shrink_clear
        move    1,5(011)
        move    2,016
        lsh     2,2
        add     2,016
        movn    2,2
        lsh     1,0(2)
        andi    1,037
        move    3,011
        pushj   17,d6fs_resize_decode_extent
        camle   013,015
        move    013,015
        move    1,010
        move    2,016
        move    3,012
        move    4,013
        pushj   17,d6fs_provider_set_extent
        sub     015,013
        aoja    016,d6fs_resize_shrink_loop

d6fs_resize_shrink_clear:
        move    014,016                  ; freeze retained count before clearing
d6fs_resize_shrink_clear_loop:
        cail    016,7
        jrst    d6fs_resize_publish
        move    1,010
        move    2,016
        pushj   17,d6fs_provider_clear_extent
        aoja    016,d6fs_resize_shrink_clear_loop

; Publish the new metadata atomically at the existing provider boundary.
d6fs_resize_publish:
        movem   014,(17)                 ; final extent count
        move    6,-6(17)
        move    1,(6)
        and     1,[-07761]
        move    5,014
        lsh     5,4
        ior     1,5
        movem   1,(6)
        move    3,-4(17)
        movem   3,2(6)
        move    1,d6fs_active_reader
        hrrz    2,-7(17)
        move    3,6
        pushj   17,d6fs_reader_put_fcb
        jumpe   1,d6fs_resize_published
        move    1,-1(17)
        camle   1,-2(17)                 ; growth failure needs rollback
        jrst    d6fs_resize_rollback
        jrst    d6fs_resize_fail

d6fs_resize_published:
        move    1,-1(17)
        caml    1,-2(17)
        jrst    d6fs_resize_update_info
        movei   1,-024(17)
        move    2,-1(17)
        pushj   17,d6fs_provider_free_file_tail
        jumpn   1,d6fs_resize_fail

d6fs_resize_update_info:
        move    1,-5(17)
        move    2,(17)
        movem   2,4(1)                   ; fi.extent_count
        move    2,-4(17)
        movem   2,7(1)                   ; fi.size_words
        setzm   3(1)                   ; former tail field is reserved
        setz    1,
        jrst    d6fs_resize_done

; Roll back only growth.  The working FCB describes every run committed by the
; grow path, so free_file_tail(old_blocks) releases exactly the new allocation.
d6fs_resize_rollback:
        move    6,-6(17)
        move    1,(6)
        and     1,[-0361]                ; expose current extent count to walker
        move    2,014
        lsh     2,4
        ior     1,2
        movem   1,(6)
        move    1,6
        move    2,-2(17)
        pushj   17,d6fs_provider_free_file_tail
        ; Restore the fixed 015-word on-disk prefix.
        movei   1,-024(17)
        move    2,-6(17)
        movei   3,015
        pushj   17,fs_copy_words

d6fs_resize_fail:
        seto    1,

d6fs_resize_done:
        move    010,-033(17)
        move    011,-032(17)
        move    012,-031(17)
        move    013,-030(17)
        move    014,-027(17)
        move    015,-026(17)
        move    016,-025(17)
        sub     17,[036,,036]
        popj    17,
