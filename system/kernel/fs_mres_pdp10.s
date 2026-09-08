; fs_mres_pdp10.s -- fixed resident bridge to optional filesystem MRES.
        .text
        .globl fs_provider_call
        .globl fs_memfs_service_jump
        .globl fs_dtfs_service_jump
        .globl fs_d6fs_service_jump
        .globl fs_mres_no_service

        .globl diskset_runtime_call
        .globl diskset_runtime_service_jump

; Stable KCORE bridge to the movable DISKSET MRES dispatcher.
; AC1 points at struct diskset_mres_request.
diskset_runtime_call:
        xct     diskset_runtime_service_jump
diskset_runtime_service_jump:
        jrst    fs_mres_no_service

; int fs_provider_call(provider, request)
; Provider topology is frozen by MINIT.  The three JRST words below are
; patched once at boot, avoiding resident service pointers and indirect calls.
fs_provider_call:
        jumpe   2,fs_mres_no_service
        subi    1,4
        jumpl   1,fs_mres_no_service
        caile   1,2
        jrst    fs_mres_no_service
        move    3,1
        move    1,2
        xct     fs_memfs_service_jump(3)
fs_memfs_service_jump:
        jrst    fs_mres_no_service
fs_dtfs_service_jump:
        jrst    fs_mres_no_service
fs_d6fs_service_jump:
        jrst    fs_mres_no_service
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
        jrst    pdp10_ret_zero
        aoj     1,
        aoj     2,
        sojg    3,fs_words_equal_loop
fs_words_equal_yes:
pdp10_ret_one:
        movei   1,1
        popj    17,

        .globl  pdp10_ret_one
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
; provider entry or zero for unsupported operations.  Request words a..e map
; to AC1..AC5, which covers every DAIMOS filesystem provider operation.
fs_mres_vector_dispatch:
        jumpe   1,fs_mres_no_service
        jumpe   2,fs_mres_no_service
        move    3,(1)                   ; op
        jumple  3,fs_mres_no_service
        camle   3,(2)
        jrst    fs_mres_no_service
        add     2,3
        xct     (2)                     ; entry loads AC7 or jumps to failure
        move    6,1                     ; preserve request pointer
        add     17,[1,,1]               ; reserve C arg 5
        move    5,5(6)                  ; request e => C arg 5
        movem   5,(17)
        move    1,1(6)
        move    2,2(6)
        move    3,3(6)
        move    4,4(6)
        pushj   17,(7)
        sub     17,[1,,1]
        popj    17,

        .globl  fs_mres_context_vector_dispatch
; int fs_mres_context_vector_dispatch(request, table, context)
; Provider C ABI: context is argument 1, request a..c become arguments 2..4,
; and request d/e are staged as stack arguments 5/6.
fs_mres_context_vector_dispatch:
        jumpe   1,fs_mres_no_service
        jumpe   2,fs_mres_no_service
        move    0,3                     ; context
        move    3,(1)                   ; op
        jumple  3,fs_mres_no_service
        camle   3,(2)
        jrst    fs_mres_no_service
        add     2,3
        xct     (2)                     ; entry loads AC7 or jumps to failure
        move    6,1                     ; request
        add     17,[2,,2]
        move    5,4(6)
        movem   5,(17)                  ; arg 5 = request d
        move    5,5(6)
        movem   5,-1(17)                ; arg 6 = request e
        move    1,0
        move    2,1(6)
        move    3,2(6)
        move    4,3(6)
        pushj   17,(7)
        sub     17,[2,,2]
        popj    17,
