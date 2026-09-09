; fs_mres_pdp10.s -- fixed resident bridge to optional filesystem MRES.
        .text
        .globl fs_provider_call
        .globl fs_provider_reg_call
        .globl fs_memfs_service_jump
        .globl fs_dtfs_service_jump
        .globl fs_d6fs_service_jump
        .globl fs_mres_no_service

        .globl diskset_runtime_reg_call
        .globl diskset_runtime_service_jump

; Stable KCORE register bridge to the movable DISKSET MRES dispatcher.
; C ABI: AC1=operation, AC2=a, AC3=b, AC4=c.  The movable export keeps a
; two-word legacy request entry, so its register entry is target+2.
diskset_runtime_reg_call:
        move    5,1
        move    1,2
        move    2,3
        move    3,4
        hrrz    4,diskset_runtime_service_jump
        cain    4,fs_mres_no_service
        jrst    fs_mres_no_service
        addi    4,2
        jrst    (4)
diskset_runtime_service_jump:
        jrst    fs_mres_no_service

; int fs_provider_call(provider, request)
; Legacy fixed-KCORE C bridge.  Movable provider exports also use the same
; request ABI during KINIT.  Every provider export is exactly two words and
; its register entry follows immediately, so the runtime bridge can derive the
; movable register entry from the already-republished service jump without a
; second permanent binding.
fs_provider_call:
        jumpe   2,fs_mres_no_service
        move    7,1                     ; provider
        move    1,2                     ; request

        .globl  fs_provider_request_call
; Provider export request ABI:
;   AC1       struct fs_mres_request *
;   AC7       provider number
fs_provider_request_call:
        jumpe   1,fs_mres_no_service
        move    6,1                     ; request pointer; AC0 cannot index
        move    5,5(6)                  ; request e
        move    4,4(6)                  ; request d
        move    3,3(6)                  ; request c
        move    2,2(6)                  ; request b
        move    1,1(6)                  ; request a
        move    6,(6)                   ; operation, after final pointer use

; Register provider ABI used by the private PDP-6 UUO bridge:
;   AC6       FS_MRES_OP_*
;   AC7       provider number (4 MEMFS, 5 DTFS, 6 D6FS)
;   AC1..AC5 request a..e
;
; MINIT already maintains one movable service jump per provider.  Its target
; is the two-word request wrapper, and the register entry is target+2.  Derive
; that address instead of adding another permanent republished binding.
fs_provider_reg_call:
        subi    7,4
        jumpl   7,fs_mres_no_service
        caile   7,2
        jrst    fs_mres_no_service
        hrrz    7,fs_memfs_service_jump(7)
        cain    7,fs_mres_no_service
        jrst    fs_mres_no_service
        addi    7,2
        jrst    (7)
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
; Register provider dispatcher.  AC6 is the operation, AC7 the vector table,
; and request a..e are already in AC1..AC5.  table[0] is the highest valid
; operation; table[op] loads AC7 with a direct provider entry or jumps to the
; shared failure return.  The fifth C argument is staged in one stack word.
fs_mres_vector_dispatch:
        jumple  6,fs_mres_no_service
        camle   6,(7)
        jrst    fs_mres_no_service
        add     7,6
        xct     (7)                     ; target into AC7, or failure jump
        add     17,[1,,1]               ; reserve C arg 5
        movem   5,(17)
        pushj   17,(7)
        sub     17,[1,,1]
        popj    17,

        .globl  fs_mres_context_vector_dispatch
; Context-register provider dispatcher.  AC0 is the provider context, AC6 the
; operation, AC7 the table, and request a..e are in AC1..AC5.  Provider C ABI
; receives context as argument 1, a..c as 2..4 and d/e as stack args 5/6.
fs_mres_context_vector_dispatch:
        jumple  6,fs_mres_no_service
        camle   6,(7)
        jrst    fs_mres_no_service
        add     7,6
        xct     (7)                     ; target into AC7, or failure jump
        add     17,[2,,2]
        movem   4,(17)                  ; arg 5 = request d
        movem   5,-1(17)                ; arg 6 = request e
        move    4,3
        move    3,2
        move    2,1
        move    1,0                     ; arg 1 = context
        pushj   17,(7)
        sub     17,[2,,2]
        popj    17,
