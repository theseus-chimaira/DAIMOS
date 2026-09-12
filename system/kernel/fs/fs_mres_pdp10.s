; fs_mres_pdp10.s -- fixed resident bridge to optional filesystem MRES.
        .text
        .globl fs_provider_reg_call
        .globl fs_memfs_service_jump
        .globl fs_dtfs_service_jump
        .globl fs_d6fs_service_jump
        .globl fs_mres_no_service
        .globl proc_wait_event
        .globl proc_wakeup_event

        .globl diskset_runtime_reg_call
        .globl diskset_runtime_reg_enter
        .globl diskset_runtime_service_jump

; Stable KCORE register bridge to the movable DISKSET MRES dispatcher.
; C ABI: AC1=operation, AC2=a, AC3=b, AC4=c.  The movable export keeps a
; two-word legacy request entry, so its register entry is target+2.
diskset_runtime_reg_call:
        move    5,1
        move    1,2
        move    2,3
        move    3,4

; Assembly register entry: AC5=operation, AC1..AC3=a..c.  AC4 is scratch.
; This keeps fixed KCORE callers from depending on movable DISKSET symbols.
diskset_runtime_reg_enter:
        hrrz    4,diskset_runtime_service_jump
        cain    4,fs_mres_no_service
        jrst    fs_mres_no_service
        jrst    (4)
diskset_runtime_service_jump:
        jrst    fs_mres_no_service

; Register provider ABI used by the private PDP-6 UUO bridge:
;   AC6       FS_MRES_OP_*
;   AC7       provider number (4 MEMFS, 5 DTFS, 6 D6FS)
;   AC1..AC5 request a..e
;
; MINIT already maintains one movable service jump per provider.  Its target
; is the two-word request wrapper, and the register entry is target+2.  Derive
; that address instead of adding another permanent republished binding.
fs_provider_reg_call:
        caige   7,4
        jrst    fs_mres_no_service
        caile   7,6
        jrst    fs_mres_no_service
        caie    7,4                    ; MEMFS never sleeps; no shared block
        jrst    fs_provider_serialized
        subi    7,4
        hrrz    7,fs_memfs_service_jump(7)
        cain    7,fs_mres_no_service
        jrst    fs_mres_no_service
        jrst    (7)

; DTFS and D6FS share fs_block_workspace and provider state across calls.
; A physical disk request may sleep, so another process can otherwise enter
; the provider while the first request is suspended and corrupt that state.
; Serialize those providers across the complete call.  The saved arguments
; live on the process-private kernel stack while proc_wait_event() switches.
fs_provider_serialized:
        push    17,1
        push    17,2
        push    17,3
        push    17,4
        push    17,5
        push    17,6
        push    17,7
fs_provider_lock_retry:
        skipe   fs_provider_busy
        jrst    fs_provider_lock_wait
        setom   fs_provider_busy
        jrst    fs_provider_lock_acquired
fs_provider_lock_wait:
        setzm   fs_provider_event
        skipn   fs_provider_busy
        jrst    fs_provider_lock_retry
        movei   1,fs_provider_event
        pushj   17,proc_wait_event
        jrst    fs_provider_lock_retry
fs_provider_lock_acquired:
        pop     17,7
        pop     17,6
        pop     17,5
        pop     17,4
        pop     17,3
        pop     17,2
        pop     17,1
        subi    7,4
        hrrz    7,fs_memfs_service_jump(7)
        cain    7,fs_mres_no_service
        jrst    fs_provider_locked_no_service
        pushj   17,(7)
        jrst    fs_provider_unlock
fs_provider_locked_no_service:
        hrroi   1,1
fs_provider_unlock:
        push    17,1
        setzm   fs_provider_busy
        setom   fs_provider_event
        movei   1,fs_provider_event
        pushj   17,proc_wakeup_event
        pop     17,1
        popj    17,
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
        jrst    pdp10_ret_one

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
; Filesystem provider vectors pack two 18-bit target addresses per word.
; Operations are 1..16; a zero halfword denotes an unsupported operation.
; AC6 is scratch after target selection and AC0..AC5 remain untouched.
fs_mres_vector_target:
        jumple  6,fs_mres_vector_target_none
        caile   6,020
        jrst    fs_mres_vector_target_none
        subi    6,1                     ; zero-based operation
        trne    6,1
        jrst    fs_mres_vector_target_odd
        lsh     6,-1
        add     7,6
        hlrz    7,(7)
        popj    17,
fs_mres_vector_target_odd:
        lsh     6,-1
        add     7,6
        hrrz    7,(7)
        popj    17,
fs_mres_vector_target_none:
        setz    7,
        popj    17,

; Register provider dispatcher.  AC6 is the operation, AC7 the packed vector,
; and request a..e are already in AC1..AC5.  The fifth C argument is staged in
; one stack word.
fs_mres_vector_dispatch:
        pushj   17,fs_mres_vector_target
        jumpe   7,fs_mres_no_service
        add     17,[1,,1]               ; reserve C arg 5
        movem   5,(17)
        pushj   17,(7)
        sub     17,[1,,1]
        popj    17,

        .globl  fs_mres_context_vector_dispatch
; Context-register provider dispatcher.  AC0 is the provider context, AC6 the
; operation, AC7 the packed vector, and request a..e are in AC1..AC5.  Provider
; C ABI receives context as argument 1, a..c as 2..4 and d/e as stack args 5/6.
fs_mres_context_vector_dispatch:
        pushj   17,fs_mres_vector_target
        jumpe   7,fs_mres_no_service
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

        .bss
fs_provider_busy:
        .block  1
fs_provider_event:
        .block  1
