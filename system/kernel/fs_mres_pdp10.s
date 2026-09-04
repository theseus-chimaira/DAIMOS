; fs_mres_pdp10.s -- fixed resident bridge to an optional filesystem MRES.
        .text
        .globl fs_mres_call
        .globl mres_call
        .globl diskset_service_addr
        .globl fs_provider_call
        .globl fs_memfs_service_addr
        .globl fs_dtfs_service_addr
        .globl fs_d6fs_service_addr

; Provider ids 4..6 index these three resident service addresses directly.
fs_memfs_service_addr:
        .word   0
fs_dtfs_service_addr:
        .word   0
fs_d6fs_service_addr:
        .word   0
diskset_service_addr:
        .word   0

; int fs_provider_call(provider, request)
fs_provider_call:
        jumpe   2,fs_mres_no_service
        subi    1,4
        jumpl   1,fs_mres_no_service
        cail    1,3
        jrst    fs_mres_no_service
        move    1,fs_memfs_service_addr(1)
        jrst    fs_mres_call


; int fs_mres_call(address, request)
; C args arrive in AC1,AC2.  Dispatcher expects request pointer in AC1.
fs_mres_call:
mres_call:
        move    3,1
        move    1,2
        andi    3,0777777
        jumpe   3,fs_mres_no_service
        pushj   17,(3)
        popj    17,
fs_mres_no_service:
        hrroi   1,1
        popj    17,


        .globl  fs_copy_words
; void fs_copy_words(src, dst, count)
; Shared forward word copy for resident filesystem data paths.
; AC1=src, AC2=dst, AC3=count.  AC4 is scratch.
fs_copy_words:
        jumpe   3,fs_copy_words_done
        move    4,2
        hrl     4,1
        add     2,3
        subi    2,1
        blt     4,(2)
fs_copy_words_done:
        popj    17,

        .globl  fs_words_equal
; int fs_words_equal(a, b, count)
; Return 1 when count words match, otherwise 0.
fs_words_equal:
        jumpe   3,fs_words_equal_yes
fs_words_equal_loop:
        move    4,(1)
        came    4,(2)
        jrst    fs_words_equal_no
        aoj     1,
        aoj     2,
        sojg    3,fs_words_equal_loop
fs_words_equal_yes:
        movei   1,1
        popj    17,
fs_words_equal_no:
        setz    1,
        popj    17,

        .globl  fs_zero_words
; void fs_zero_words(dst, count)
; Shared contiguous word clear.  AC1=dst, AC2=count, AC3 is scratch.
fs_zero_words:
        jumpe   2,fs_zero_words_done
        setzm   (1)
        subi    2,1
        jumpe   2,fs_zero_words_done
        move    3,1
        aoj     3,
        hrl     3,1
        add     1,2
        blt     3,(1)
fs_zero_words_done:
        popj    17,

        .globl  fs_zero_block_workspace
; Zero the shared 128-word filesystem transfer block without disturbing AC1.
fs_zero_block_workspace:
        push    17,1
        movei   1,fs_block_workspace
        movei   2,0200
        pushj   17,fs_zero_words
        pop     17,1
        popj    17,

        .globl  fs_mres_vector_dispatch
; int fs_mres_vector_dispatch(request, table)
;
; table[0] is the highest valid operation number; table[op] is a direct
; provider entry or zero for unsupported operations.  Request words a..f map
; exactly to AC1..AC6, so provider packages do not need repeated C switches.
fs_mres_vector_dispatch:
        jumpe   1,fs_mres_no_service
        jumpe   2,fs_mres_no_service
        move    3,(1)                   ; op
        jumple  3,fs_mres_no_service
        camle   3,(2)
        jrst    fs_mres_no_service
        add     2,3
        move    7,(2)
        jumpe   7,fs_mres_no_service
        move    6,1                     ; preserve request pointer
        add     17,[2,,2]               ; reserve C args 5 and 6
        move    5,5(6)                  ; request e => C arg 5
        movem   5,(17)
        move    5,6(6)                  ; request f => C arg 6
        movem   5,-1(17)
        move    1,1(6)
        move    2,2(6)
        move    3,3(6)
        move    4,4(6)
        pushj   17,(7)
        sub     17,[2,,2]
        popj    17,

        .globl  fs_mres_context_vector_dispatch
; int fs_mres_context_vector_dispatch(request, table, context)
; Provider C ABI: context is argument 1, request a..c become arguments 2..4,
; and request d..f are staged as stack arguments 5..7.
fs_mres_context_vector_dispatch:
        jumpe   1,fs_mres_no_service
        jumpe   2,fs_mres_no_service
        move    0,3                     ; context
        move    3,(1)                   ; op
        jumple  3,fs_mres_no_service
        camle   3,(2)
        jrst    fs_mres_no_service
        add     2,3
        move    7,(2)
        jumpe   7,fs_mres_no_service
        move    6,1                     ; request
        add     17,[3,,3]
        move    5,4(6)
        movem   5,(17)                  ; arg 5 = request d
        move    5,5(6)
        movem   5,-1(17)                ; arg 6 = request e
        move    5,6(6)
        movem   5,-2(17)                ; arg 7 = request f
        move    1,0
        move    2,1(6)
        move    3,2(6)
        move    4,3(6)
        pushj   17,(7)
        sub     17,[3,,3]
        popj    17,
