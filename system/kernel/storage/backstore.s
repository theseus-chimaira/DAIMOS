; backstore.s -- compact authoritative backing-store allocator.
;
; Process swap and MEMFS share one direct in-RAM allocation bitmap.  One bit
; represents one 0200-word backing block; set means allocated.  Allocation is
; first-fit and preserves a caller-specified free-block reserve.  This is the
; sole implementation: the bitmap is transient runtime state and no filesystem
; or backing-device policy lives here.
;
; The implementation uses only the common PDP-6/PDP-10 instruction subset.
; Backstore sizes are bounded non-negative block counts, so normal signed
; compare instructions are sufficient and avoid generic unsigned helpers.

        .text
        .globl  backstore_bitmap_words
        .globl  backstore_init
        .globl  backstore_alloc
        .globl  backstore_free

; unsigned int backstore_bitmap_words(kword_t blocks)
backstore_bitmap_words:
        addi    1,043                  ; ceil(blocks / 36)
        idivi   1,044
        popj    17,

; void backstore_init(kword_t *bitmap, kword_t blocks)
backstore_init:
        movem   1,backstore_bitmap
        movem   2,backstore_blocks
        setzm   backstore_blocks_used
        setzm   backstore_enabled
        popj    17,

; int backstore_alloc(kword_t blocks, kword_t reserve, kword_t *firstp)
; AC10=run length, AC11=reserve, AC12=result pointer, AC13=candidate,
; AC14=offset within candidate.  The saved ACs satisfy the C call ABI.
backstore_alloc:
        skipn   backstore_enabled
        jrst    backstore_alloc_fail_fast
        jumpe   1,backstore_alloc_fail_fast
        jumpe   3,backstore_alloc_fail_fast
        skipn   backstore_bitmap
        jrst    backstore_alloc_fail_fast
        camle   1,backstore_blocks
        jrst    backstore_alloc_fail_fast

        move    4,backstore_blocks
        sub     4,backstore_blocks_used
        camg    4,2                    ; free <= reserve
        jrst    backstore_alloc_fail_fast
        sub     4,2                    ; capacity available after reserve
        camle   1,4
        jrst    backstore_alloc_fail_fast

        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1
        move    011,2
        move    012,3
        setz    013,

backstore_alloc_scan:
        move    4,backstore_blocks
        sub     4,010                  ; highest legal first block
        camle   013,4
        jrst    backstore_alloc_fail
        setz    014,

backstore_alloc_test:
        caml    014,010                ; offset >= run length => free run
        jrst    backstore_alloc_found
        move    1,013
        add     1,014
        idivi   1,044                  ; AC1=bitmap word, AC2=bit 0..35
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        tdne    3,(1)                  ; clear bit skips occupied branch
        jrst    backstore_alloc_used
        aoja    014,backstore_alloc_test

backstore_alloc_used:
        add     013,014                ; skip free prefix plus occupied block
        aoja    013,backstore_alloc_scan

backstore_alloc_found:
        setz    014,
backstore_alloc_mark:
        caml    014,010
        jrst    backstore_alloc_done
        move    1,013
        add     1,014
        idivi   1,044
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        move    4,(1)
        ior     4,3
        movem   4,(1)
        aoja    014,backstore_alloc_mark

backstore_alloc_done:
        addm    010,backstore_blocks_used
        movem   013,(012)
        setz    1,
        jrst    backstore_alloc_restore
backstore_alloc_fail:
        seto    1,
backstore_alloc_restore:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
backstore_alloc_fail_fast:
        seto    1,
        popj    17,

; void backstore_free(kword_t first, kword_t blocks)
backstore_free:
        jumpe   2,backstore_free_return
        caml    1,backstore_blocks      ; first >= total blocks
        jrst    backstore_free_return
        move    3,backstore_blocks
        sub     3,1                     ; blocks available from first
        camle   2,3
        jrst    backstore_free_return

        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        setz    012,
backstore_free_loop:
        caml    012,011
        jrst    backstore_free_count
        move    1,010
        add     1,012
        idivi   1,044
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        setcm   3,3
        move    4,(1)
        and     4,3
        movem   4,(1)
        aoja    012,backstore_free_loop

backstore_free_count:
        movn    3,011
        addm    3,backstore_blocks_used
        pop     17,012
        pop     17,011
        pop     17,010
backstore_free_return:
        popj    17,

        .bss
        .globl  backstore_bitmap
        .globl  backstore_blocks
        .globl  backstore_blocks_used
        .globl  backstore_enabled
backstore_bitmap:       .block 1
backstore_blocks:       .block 1
backstore_blocks_used:  .block 1
backstore_enabled:      .block 1
