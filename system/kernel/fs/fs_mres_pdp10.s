; fs_mres_pdp10.s -- fixed resident bridge to optional filesystem MRES.
        .text
        .globl fs_provider_reg_call
        .globl fs_memfs_service_jump
        .globl fs_dtfs_service_jump
        .globl fs_d6fs_service_jump
        .globl fs_tsfs_service_jump
        .globl fs_mres_no_service
        .globl proc_wait_event
        .globl proc_wakeup_event

        .globl blockset_runtime_reg_call
        .globl blockset_runtime_reg_enter
        .globl blockset_runtime_service_jump
        .globl blockset_direct_configure
        .globl dsk270_read_sector
        .globl dsk270_write_sector

; Stable KCORE register bridge to the movable BLOCKSET MRES dispatcher.
; C ABI: AC1=operation, AC2=a, AC3=b, AC4=c.  The movable export keeps a
; two-word legacy request entry, so its register entry is target+2.
blockset_runtime_reg_call:
        move    5,1
        move    1,2
        move    2,3
        move    3,4

; Assembly register entry: AC5=operation, AC1..AC3=a..c.  AC4 is scratch.
; This keeps fixed KCORE callers from depending on movable BLOCKSET symbols.
blockset_runtime_reg_enter:
        cain    5,6                    ; BLOCKSET_MRES_OP_TAIL_BLOCKS
        jrst    blockset_direct_tail_blocks
        cain    5,7                    ; BLOCKSET_MRES_OP_TAIL_READ
        jrst    blockset_direct_tail_read
        cain    5,010                  ; BLOCKSET_MRES_OP_TAIL_WRITE
        jrst    blockset_direct_tail_write
        hrrz    4,blockset_runtime_service_jump
        cain    4,fs_mres_no_service
        jrst    fs_mres_no_service
        jrst    (4)
blockset_runtime_service_jump:
        jrst    fs_mres_no_service

; Root swap-tail service shared by singleton and multi-member BLOCKSET roots.
; direct_map/data describe the singleton fast path.  direct_tail is always the
; total logical tail span.  When BLOCKSET is installed, op 10 maps each tail
; logical block; otherwise the singleton arithmetic below is used directly.
blockset_direct_configure:
        move    5,1
        lsh     5,022
        ior     5,2
        movem   5,blockset_direct_map
        movem   3,blockset_direct_blocks
        movem   4,blockset_direct_tail
        popj    17,

blockset_direct_tail_read:
        setz    4,
        jrst    blockset_direct_tail_io
blockset_direct_tail_write:
        movei   4,1

blockset_direct_tail_io:
        jumpe   2,pdp10_ret_zero
        jumpe   3,pdp10_ret_neg1
        jumpl   1,pdp10_ret_neg1
        jumpl   2,pdp10_ret_neg1
        move    6,blockset_direct_tail
        jumpe   6,pdp10_ret_neg1
        caml    1,6
        jrst    pdp10_ret_neg1
        sub     6,1
        camle   2,6
        jrst    pdp10_ret_neg1
        aos     mfsdev_d6set_reads(4)
        addm    2,mfsdev_d6set_blocks_read(4)
        add     17,[6,,6]
        movei   0,-5(17)
        hrli    0,010
        blt     0,(17)
        move    010,3                  ; buffer
        move    011,1                  ; logical tail block
        move    012,2                  ; count
        move    013,4                  ; write flag

blockset_direct_tail_loop:
        hrrz    4,blockset_runtime_service_jump
        cain    4,fs_mres_no_service
        jrst    blockset_direct_tail_single
        move    1,011
        movei   5,012                  ; BLOCKSET_MRES_OP_TAIL_MAP
        pushj   17,(4)
        jumpl   1,blockset_direct_tail_done
        jrst    blockset_direct_tail_mapped

blockset_direct_tail_single:
        hlrz    1,blockset_direct_map
        hrrz    2,blockset_direct_map
        add     2,blockset_direct_blocks
        add     2,011

blockset_direct_tail_mapped:
        move    3,010
        jumpe   013,blockset_direct_tail_read_one
        pushj   17,dsk270_write_sector
        jrst    blockset_direct_tail_after_one
blockset_direct_tail_read_one:
        pushj   17,dsk270_read_sector
blockset_direct_tail_after_one:
        jumpe   1,blockset_direct_tail_after_ok
        aos     mfsdev_storage_errors+4
        jrst    blockset_direct_tail_done
blockset_direct_tail_after_ok:
        addi    010,0200
        aoj     011,
        sojn    012,blockset_direct_tail_loop
        setz    1,
blockset_direct_tail_done:
        movei   0,010
        hrli    0,-5(17)
        blt     0,013
        sub     17,[6,,6]
        popj    17,
blockset_direct_tail_blocks:
        move    1,blockset_direct_tail
        popj    17,

; Register provider ABI used by the private PDP-6 UUO bridge:
;   AC6       FS_MRES_OP_*
;   AC7       provider number (4 MEMFS, 5 DTFS, 6 D6FS, 7 TSFS)
;   AC1..AC5 request a..e
;
; MINIT already maintains one movable service jump per provider.  Its target
; is the two-word request wrapper, and the register entry is target+2.  Derive
; that address instead of adding another permanent republished binding.
fs_provider_reg_call:
        caige   7,4
        jrst    fs_mres_no_service
        caile   7,7
        jrst    fs_mres_no_service
        caie    7,4                    ; MEMFS never sleeps; no shared block
        jrst    fs_provider_serialized
        subi    7,4
        hrrz    7,fs_memfs_service_jump(7)
        cain    7,fs_mres_no_service
        jrst    fs_mres_no_service
        jrst    (7)

; DTFS, TSFS and D6FS share fs_block_workspace and provider state across calls.
; A physical disk request may sleep, so another process can otherwise enter
; the provider while the first request is suspended and corrupt that state.
; Serialize those providers across the complete call.  The saved arguments
; live on the process-private kernel stack while proc_wait_event() switches.
fs_provider_serialized:
        ; Keep the original request registers on the process-private stack
        ; for the entire lock wait.  A resumed waiter may lose live scratch
        ; ACs while another process owns the provider.
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,1
        blt     0,(17)
fs_provider_lock_retry:
        skipn   fs_provider_ready
        jrst    fs_provider_lock_wait
        setzm   fs_provider_ready
        movei   0,1
        hrli    0,-6(17)
        blt     0,7
        sub     17,[7,,7]
        subi    7,4
        hrrz    7,fs_memfs_service_jump(7)
        cain    7,fs_mres_no_service
        jrst    fs_provider_locked_no_service
        pushj   17,(7)
        jrst    fs_provider_unlock

; The executive is not process-preemptible between the ready test and clear.
fs_provider_lock_wait:
        movei   1,fs_provider_ready
        pushj   17,proc_wait_event
        jrst    fs_provider_lock_retry

fs_provider_locked_no_service:
        hrroi   1,1
fs_provider_unlock:
        push    17,1
        setom   fs_provider_ready
        movei   1,fs_provider_ready
        pushj   17,proc_wakeup_event
        pop     17,1
        popj    17,
fs_memfs_service_jump:
        jrst    fs_mres_no_service
fs_dtfs_service_jump:
        jrst    fs_mres_no_service
fs_d6fs_service_jump:
        jrst    fs_mres_no_service
fs_tsfs_service_jump:
        jrst    fs_mres_no_service

; KCORE memory-pressure bridge to the movable D6FS clean-cache reclaimer.
; AC1=requested allocation size; return number of released cache slabs.
        .globl  fs_d6fs_cache_reclaim
        .globl  d6fs_cache_reclaim_jump
fs_d6fs_cache_reclaim:
        hrrz    4,d6fs_cache_reclaim_jump
        cain    4,fs_mres_no_service
        jrst    pdp10_ret_zero
        jrst    (4)
d6fs_cache_reclaim_jump:
        jrst    fs_mres_no_service
fs_mres_no_service:
        jrst    pdp10_ret_neg1

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

        .globl  fs_move_words
; void fs_move_words(src, dst, count)
; Overlap-safe resident word move.  Forward/non-overlapping copies retain the
; BLT fast path; an upward overlapping move walks backwards.
; AC1=src, AC2=dst, AC3=count.  AC4 is scratch.
fs_move_words:
        jumpe   3,fs_move_words_done
        move    4,2
        sub     4,1                    ; delta = dst - src
        jumple  4,fs_copy_words       ; dst <= src: forward copy is safe
        sub     4,3
        jumpge  4,fs_copy_words       ; dst >= src + count: no overlap
        add     1,3
        subi    1,1
        add     2,3
        subi    2,1
fs_move_words_back:
        move    4,(1)
        movem   4,(2)
        subi    1,1
        subi    2,1
        sojg    3,fs_move_words_back
fs_move_words_done:
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

        .data
fs_provider_ready:
        .word   1

        .bss
blockset_direct_map:    .block 1
blockset_direct_blocks: .block 1
blockset_direct_tail:   .block 1
