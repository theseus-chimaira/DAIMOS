        .text
        .globl  d6fs_provider_write_dirent
        .globl  pclk_time36
; int d6fs_provider_write_dirent(dir, slot, di)
;
; The directory FCB has already passed d6fs_provider_fcb validation.  D6FS
; geometry keeps both the directory offset and size below the signed PDP-10
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
        movei   5,4                      ; directory entries use four chars/word
        movem   5,(17)                   ; fifth argument: tail
        move    1,010
        movei   2,-040(17)
        movei   3,-020(17)
        pushj   17,d6fs_provider_resize_fcb
        jumpn   1,d6fs_provider_write_dirent_fail

d6fs_provider_write_dirent_store:
        movei   5,6
        movem   5,(17)                   ; fifth argument: nwords
        movei   1,d6fs_provider_reader
        movei   2,-040(17)
        move    3,011
        movei   4,-6(17)
        pushj   17,d6fs_reader_write_words
        caie    1,6
        jrst    d6fs_provider_write_dirent_fail
        pushj   17,pclk_time36
        movem   1,-035(17)               ; directory FCB MTIME: -040 + 3
        movei   1,d6fs_provider_reader
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
; table capacity), so a direct PDP-10 comparison is sufficient.  The scratch
; frame is exactly one FCB plus one decoded-info object; only AC010/AC011 live
; across d6fs_reader_fcb calls.
d6fs_provider_free_fcb:
        add     17,[034,,034]
        movem   010,-033(17)
        movem   011,-032(17)
        move    010,1                    ; indexp
        movei   011,1                    ; FCB zero is reserved

d6fs_provider_free_fcb_loop:
        caml    011,d6fs_provider_reader+011
        jrst    d6fs_provider_free_fcb_fail
        movei   1,d6fs_provider_reader
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
