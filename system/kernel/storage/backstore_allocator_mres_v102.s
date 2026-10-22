; backstore_allocator_mres_v102.s -- pinned runtime backing allocation.
;
; KCORE retains the state words, boot initializer, bitmap-size helper and
; one-word callable ABI stubs. MINIT installs this package before process
; boot; the package is pinned because swap and MEMFS may use it at any time.
; Keeping the bitmap in KCORE also lets transient KINIT initialize it
; without a permanent duplicate or module ownership ambiguity.
        .text
        .globl backstore_bitmap
        .globl backstore_blocks
        .globl backstore_blocks_used
        .globl backstore_enabled
        .globl backstore_alloc_mres_v102
        .globl backstore_free_mres_v102

; int backstore_alloc_mres_v102(kword_t blocks, kword_t reserve, kword_t *firstp)
; AC10=run length, AC11=reserve, AC12=result pointer, AC13=candidate,
; AC14=offset within candidate.  The saved ACs satisfy the C call ABI.
backstore_alloc_mres_v102:
        skipn   backstore_enabled
        jrst    backstore_alloc_mres_v102_fail_fast
        jumpe   1,backstore_alloc_mres_v102_fail_fast
        jumpe   3,backstore_alloc_mres_v102_fail_fast
        skipn   backstore_bitmap
        jrst    backstore_alloc_mres_v102_fail_fast
        camle   1,backstore_blocks
        jrst    backstore_alloc_mres_v102_fail_fast

        move    4,backstore_blocks
        sub     4,backstore_blocks_used
        camg    4,2                    ; free <= reserve
        jrst    backstore_alloc_mres_v102_fail_fast
        sub     4,2                    ; capacity available after reserve
        camle   1,4
        jrst    backstore_alloc_mres_v102_fail_fast

        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1
        move    011,2
        move    012,3
        setz    013,

backstore_alloc_mres_v102_scan:
        move    4,backstore_blocks
        sub     4,010                  ; highest legal first block
        camle   013,4
        jrst    backstore_alloc_mres_v102_fail
        setz    014,

backstore_alloc_mres_v102_test:
        caml    014,010                ; offset >= run length => free run
        jrst    backstore_alloc_mres_v102_found
        move    1,013
        add     1,014
        idivi   1,044                  ; AC1=bitmap word, AC2=bit 0..35
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        tdne    3,(1)                  ; clear bit skips occupied branch
        jrst    backstore_alloc_mres_v102_used
        aoja    014,backstore_alloc_mres_v102_test

backstore_alloc_mres_v102_used:
        add     013,014                ; skip free prefix plus occupied block
        aoja    013,backstore_alloc_mres_v102_scan

backstore_alloc_mres_v102_found:
        setz    014,
backstore_alloc_mres_v102_mark:
        caml    014,010
        jrst    backstore_alloc_mres_v102_done
        move    1,013
        add     1,014
        idivi   1,044
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        move    4,(1)
        ior     4,3
        movem   4,(1)
        aoja    014,backstore_alloc_mres_v102_mark

backstore_alloc_mres_v102_done:
        addm    010,backstore_blocks_used
        movem   013,(012)
        setz    1,
        jrst    backstore_alloc_mres_v102_restore
backstore_alloc_mres_v102_fail:
        seto    1,
backstore_alloc_mres_v102_restore:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
backstore_alloc_mres_v102_fail_fast:
        seto    1,
        popj    17,

; void backstore_free_mres_v102(kword_t first, kword_t blocks)
backstore_free_mres_v102:
        jumpe   2,backstore_free_mres_v102_return
        caml    1,backstore_blocks      ; first >= total blocks
        jrst    backstore_free_mres_v102_return
        move    3,backstore_blocks
        sub     3,1                     ; blocks available from first
        camle   2,3
        jrst    backstore_free_mres_v102_return

        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        setz    012,
backstore_free_mres_v102_loop:
        caml    012,011
        jrst    backstore_free_mres_v102_count
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
        aoja    012,backstore_free_mres_v102_loop

backstore_free_mres_v102_count:
        movn    3,011
        addm    3,backstore_blocks_used
        pop     17,012
        pop     17,011
        pop     17,010
backstore_free_mres_v102_return:
        popj    17,

