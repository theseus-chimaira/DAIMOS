/**
 * @file vm_pdp6.s
 * @brief PDP-6 APR activation, user mapping, and no-return user-entry paths.
 *
 * Generic kernel code treats struct proc word 1 as opaque VM state except for
 * its LH logical-space size. This backend stores the 02000-word-aligned
 * physical relocation base in RH and programs the PDP-6 APR relocation/
 * protection word directly. These routines are machine-specific and therefore
 * correctly retain the _pdp6 suffix.
 */

        .text
        .globl  vm_user_words
        .globl  vm_user_mapping_hold
        .globl  vm_user_mapping_release
        .globl  vm_activate_current
        .globl  vm_space_brk_current
        .globl  vm_enter_initial_user
        .globl  vm_space_startup
        .globl  vm_space_load_file
        .globl  vm_space_create
        .globl  vm_space_destroy
        .globl  vm_extent_move
        .globl  fs_copy_words
        .globl  fs_move_words
        .globl  fs_zero_words
        .globl  mm_alloc_aligned
        .globl  mm_free
        .globl  mm_extents
        .globl  mm_arenas
        .globl  mm_extent_count
        .globl  mm_arena_count
        .globl  proc_swap_detach
        .globl  vfs_read_words
        .globl  proc_runq_add
        .globl  proc_runq_remove
        .globl  proc_current_slot
        .globl  proc_current_ptr
        .globl  proc_table
        .globl  proc_slot_ptr
        .globl  proc_record_kernel_sp
        .globl  mach_kernel_sp
        .globl  kret_zero

/**
 * @brief Translate one logical user address into the current physical mapping.
 * @param AC1 Logical user word address.
 * @return AC1 mapped executive address or zero; AC3 logical end, AC4 bias.
 */
vm_user_words:
        hrrz    1,1
        caige   1,020
        jrst    kret_zero
        hlrz    3,vm_pdp6_apr
        addi    3,02000
        caml    1,3
        jrst    kret_zero
        hrrz    4,vm_pdp6_apr
        add     1,4
        popj    17,

; Mark the current process as holding a translated physical user mapping.
; Preserve AC0 and AC5 so callers can bracket an existing mapped argument
; without disturbing the native syscall ABI.  The bit lives in the already
; resident u-area control word and therefore adds no per-process storage.
vm_user_mapping_hold:
        push    17,0
        push    17,5
        move    5,proc_current_ptr
        hlrz    0,(5)
        jumpe   0,vm_user_mapping_hold_done
        move    5,0                     ; AC0 cannot be an index register
        move    0,0024(5)
        iori    0,02
        movem   0,0024(5)
vm_user_mapping_hold_done:
        pop     17,5
        pop     17,0
        popj    17,

; Clear only the mapping bit.  ANDI would also zero the left half of the packed
; control word and destroy TTY/stop/report state, so use ANDCMI deliberately.
vm_user_mapping_release:
        push    17,0
        push    17,5
        move    5,proc_current_ptr
        hlrz    0,(5)
        jumpe   0,vm_user_mapping_release_done
        move    5,0                     ; AC0 cannot be an index register
        move    0,0024(5)
        andcmi  0,02
        movem   0,0024(5)
vm_user_mapping_release_done:
        pop     17,5
        pop     17,0
        popj    17,

/** @brief Program the PDP-6 APR for the current process's resident VM. */
vm_activate_current:
        move    1,proc_current_ptr
        move    2,1(1)                  ; user words,,physical base
        sub     2,[02000,,0]            ; APR LH stores words-02000
        movem   2,vm_pdp6_apr
        datao   0000,vm_pdp6_apr
        popj    17,

/*
 * @brief Query or set the current process break and resize its PDP-6 extent.
 * @param AC1 Zero to query, otherwise the requested logical break.
 * @return AC1 Current/new break, or -1 on validation/allocation failure.
 *
 * The active process must never expose a new MM descriptor/base while the APR
 * still describes the old extent.  Keep PI disabled across every in-place
 * descriptor change or image move, publish vm_state and the break word, load
 * APR, and only then restore PI.  High-placed processes normally grow into the
 * free gap below themselves; fs_move_words() makes that overlap-safe and avoids
 * requiring two complete compiler images in core.  A separate larger extent
 * is only the fragmentation fallback.
 */
vm_space_brk_current:
        ; Thirteen-word frame: saved AC10..AC16 at -014..-006 and six locals.
        ; Locals: floor -005, extent -004, PI -003, arena-low -002,
        ; arena-high -001, new-base 0.
        add     17,[015,,015]
        movei   0,-014(17)
        hrli    0,10
        blt     0,-006(17)
        move    10,1                    ; requested break
        move    11,proc_current_ptr
        jumpe   11,vm_brk_fail
        move    4,(11)
        trnn    4,0400000               ; PROC_F_UAREA
        jrst    vm_brk_fail
        hrrz    14,1(11)                ; old physical base
        jumpe   14,vm_brk_fail
        hlrz    13,1(11)                ; old allocated words
        jumpe   13,vm_brk_fail
        hlrz    12,(11)                 ; stable u-area base
        move    4,0407(12)              ; PROC_BRK_OFFSET: floor,,current
        hlrz    5,4
        movem   5,-005(17)
        jumpn   10,vm_brk_set
        hrrz    1,4
        jrst    vm_brk_return

vm_brk_set:
        tlne    10,0777777              ; break must be an 18-bit address
        jrst    vm_brk_fail
        move    5,10
        move    6,-005(17)
        camge   5,6                     ; never below initial image/stack floor
        jrst    vm_brk_fail
        move    15,10
        addi    15,01777
        tlne    15,0777777              ; rounding overflowed 18-bit space
        jrst    vm_brk_fail
        andi    15,0776000              ; APR allocation quantum = 02000 words
        jumpe   15,vm_brk_fail
        came    15,13
        jrst    vm_brk_resize

        ; No APR/MM change: publish only the exact byte/character break.
vm_brk_publish_only:
        hrlz    4,-005(17)
        hrr     4,10
        movem   4,0407(12)
        move    1,10
        jrst    vm_brk_return

vm_brk_resize:
        ; Freeze scheduler/MM observers while the current mapping transaction
        ; is represented inconsistently by the descriptor table and APR.
        coni    0004,1
        movem   1,-003(17)
        cono    0004,000400

        ; Find and validate the current MM_TYPE_PROCESS descriptor.
        setz    16,
vm_brk_find_extent:
        caml    16,mm_extent_count
        jrst    vm_brk_fail_pi
        move    4,16
        lsh     4,1
        hrrz    5,mm_extents(4)
        camn    5,14
        jrst    vm_brk_extent_found
        aoja    16,vm_brk_find_extent
vm_brk_extent_found:
        movei   6,mm_extents(4)
        movem   6,-004(17)
        hlrz    5,1(6)
        andi    5,7
        caie    5,1                     ; MM_TYPE_PROCESS
        jrst    vm_brk_fail_pi
        hrrz    5,1(6)
        came    5,proc_current_slot
        jrst    vm_brk_fail_pi
        move    5,1(6)
        tlne    5,03770                 ; physical pin count
        jrst    vm_brk_fail_pi
        hlrz    5,(6)
        came    5,13
        jrst    vm_brk_fail_pi

        ; Shrink keeps the physical base and immediately releases the tail gap.
        camg    15,13
        jrst    vm_brk_shrink

        ; Locate the managed arena containing the complete old extent.
        setz    1,
vm_brk_find_arena:
        caml    1,mm_arena_count
        jrst    vm_brk_fail_pi
        move    2,mm_arenas(1)
        hrrz    3,2                     ; arena low
        hlrz    4,2
        add     4,3                     ; arena high (exclusive)
        caml    14,3
        jrst    vm_brk_arena_low_ok
        aoja    1,vm_brk_find_arena
vm_brk_arena_low_ok:
        move    5,14
        add     5,13
        camle   5,4
        aoja    1,vm_brk_find_arena
        movem   3,-002(17)
        movem   4,-001(17)

        ; Try the following free gap first so no relocation/copy is required.
        move    7,4                     ; next-base defaults to arena high
        move    1,16
        aoj     1,
        caml    1,mm_extent_count
        jrst    vm_brk_have_next
        lsh     1,1
        hrrz    2,mm_extents(1)
        camge   2,4
        move    7,2
vm_brk_have_next:
        move    5,14
        add     5,15                    ; desired same-base end
        camle   5,7
        jrst    vm_brk_try_down
        move    1,14
        add     1,13                    ; newly exposed physical tail
        move    2,15
        sub     2,13
        pushj   17,fs_zero_words
        move    5,14
        movem   5,(17)                  ; new-base local
        jrst    vm_brk_rebase_extent

vm_brk_try_down:
        ; Previous extent end, or arena low if this is its first extent.
        move    7,-002(17)
        skipg   16
        jrst    vm_brk_have_prev
        move    1,16
        subi    1,1
        lsh     1,1
        hrrz    2,mm_extents(1)
        caml    2,-002(17)
        jrst    vm_brk_prev_same_arena
        jrst    vm_brk_have_prev
vm_brk_prev_same_arena:
        hlrz    3,mm_extents(1)
        add     2,3
        move    7,2
vm_brk_have_prev:
        move    6,15
        sub     6,13                    ; delta
        move    5,14
        sub     5,6                     ; candidate new base
        jumpl   5,vm_brk_fallback
        camge   5,7
        jrst    vm_brk_fallback
        movem   5,(17)
        move    1,14
        move    2,5
        move    3,13
        pushj   17,fs_move_words
        move    1,(17)
        add     1,13
        move    2,15
        sub     2,13
        pushj   17,fs_zero_words
        jrst    vm_brk_rebase_extent

vm_brk_shrink:
        move    5,14
        movem   5,(17)

vm_brk_rebase_extent:
        ; The extent remains between the same neighbors for same-base,
        ; shrink, and downward-in-gap growth, so its sorted slot is unchanged.
        hrlz    4,15
        hrr     4,(17)
        move    6,-004(17)
        movem   4,(6)
        jrst    vm_brk_commit

vm_brk_fallback:
        ; No adjacent room.  Restore PI while the general allocator may reclaim,
        ; compact or swap other processes, then allocate a complete replacement.
        move    1,-003(17)
        trne    1,000200
        cono    0004,000200
        movei   1,(17)                  ; sixth arg: &new_base
        push    17,1
        push    17,[1]                  ; fifth arg: MM_ALLOC_HIGH
        move    1,15
        movei   2,02000
        movei   3,1                     ; MM_TYPE_PROCESS
        move    4,proc_current_slot
        pushj   17,mm_alloc_aligned
        sub     17,kconst_2_2
        jumpn   1,vm_brk_fail

        ; Re-enter a critical section for copy, old-descriptor removal and APR
        ; publication.  The current process itself is excluded from swap/move.
        coni    0004,1
        movem   1,-003(17)
        cono    0004,000400
        move    1,14
        move    2,(17)
        move    3,13
        pushj   17,fs_copy_words
        move    1,(17)
        add     1,13
        move    2,15
        sub     2,13
        pushj   17,fs_zero_words
        move    1,14
        movei   2,1
        move    3,proc_current_slot
        pushj   17,mm_free
        jumpe   1,vm_brk_commit

        ; Old mapping still owns the running process.  Release the unused new
        ; extent and return failure without changing vm_state/APR.
        move    1,(17)
        movei   2,1
        move    3,proc_current_slot
        pushj   17,mm_free
        jrst    vm_brk_fail_pi

vm_brk_commit:
        ; Publish logical allocation and exact break before loading APR.  PI is
        ; still disabled, so no observer can see a half-committed mapping.
        hrlz    4,15
        hrr     4,(17)
        movem   4,1(11)
        hrlz    5,-005(17)
        hrr     5,10
        movem   5,0407(12)

        ; Prepare caller-visible result, APR word, and prior PI state in AC1-3.
        move    1,10
        move    2,15
        subi    2,02000
        hrl     2,2
        hrr     2,(17)
        move    3,-003(17)
        jrst    vm_brk_return_commit

vm_brk_fail_pi:
        move    1,-003(17)
        trne    1,000200
        cono    0004,000200
vm_brk_fail:
        seto    1,
vm_brk_return:
        movei   0,10
        hrli    0,-014(17)
        blt     0,16
        sub     17,[015,,015]
        popj    17,

vm_brk_return_commit:
        ; Restore the C ABI before changing APR.  After DATAO there are no
        ; nested subroutine returns: restore PI and return directly to the
        ; monitor-UUO boundary, matching the proven PDP-6 path.
        movei   0,10
        hrli    0,-014(17)
        blt     0,16
        sub     17,[015,,015]
        movem   2,vm_pdp6_apr
        datao   0000,vm_pdp6_apr
        trne    3,000200
        cono    0004,000200
        popj    17,

/**
 * @brief Allocate and clear one aligned contiguous PDP-6 user extent.
 *
 * @param AC1 Process descriptor.
 * @param AC2 MM owner/process slot.
 * @param AC3 Requested words.
 * @return AC1 zero on success, -1 on allocation failure.
 */
vm_space_create:
        push    17,10
        push    17,11
        push    17,12
        move    10,1                   ; descriptor
        move    11,2                   ; owner
        move    12,3
        addi    12,01777
        and     12,[-02000]            ; round to 02000-word boundary

        push    17,[0]                 ; local allocation base
        movei   1,(17)
        push    17,1                   ; sixth arg: basep
        push    17,[1]                 ; fifth arg: MM_ALLOC_HIGH
        move    1,12
        movei   2,02000
        movei   3,1                    ; MM_TYPE_PROCESS
        move    4,11
        pushj   17,mm_alloc_aligned
        sub     17,kconst_2_2
        jumpn   1,vm_space_create_fail

        move    1,(17)
        move    2,12
        pushj   17,fs_zero_words
        hrlz    1,12
        hrr     1,(17)
        movem   1,1(10)
        setz    1,
        jrst    vm_space_create_done
vm_space_create_fail:
        seto    1,
vm_space_create_done:
        sub     17,kconst_1_1
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,

/**
 * @brief Release one resident user extent and clear its VM state.
 *
 * @param AC1 Process descriptor.
 * @param AC2 MM owner/process slot.
 * @return AC1 zero on success, -1 if MM rejected the free.
 */
vm_space_destroy:
        push    17,1                    ; descriptor
        push    17,2                    ; owner
        hrrz    3,1(1)
        jumpe   3,vm_space_destroy_detach
        move    1,3
        movei   2,1                    ; MM_TYPE_PROCESS
        move    3,(17)
        pushj   17,mm_free
        jumpn   1,vm_space_destroy_fail
vm_space_destroy_detach:
        move    1,(17)
        pushj   17,proc_swap_detach
        move    2,-1(17)
        setom   1(2)                   ; VM_SPACE_NONE
        setz    1,
        jrst    vm_space_destroy_done
vm_space_destroy_fail:
        seto    1,
vm_space_destroy_done:
        sub     17,kconst_2_2
        popj    17,

/**
 * @brief Read executable words directly into one logical user-space offset.
 *
 * @param AC1 Process descriptor.
 * @param AC2 Executable vnode.
 * @param AC3 File word offset.
 * @param AC4 Logical user word offset.
 * @param -1(AC17) Requested word count (fifth C argument).
 * @return AC1 zero only when the provider returned the complete word count.
 */
vm_space_load_file:
        move    6,2                    ; vnode
        move    7,3                    ; file offset
        hrrz    5,1(1)                 ; physical relocation base
        add     5,4                    ; physical destination
        move    4,-1(17)               ; requested words
        move    1,6
        move    2,7
        move    3,5
        pushj   17,vfs_read_words
        came    1,-1(17)
        jrst    kret_neg1
        jrst    kret_zero

/**
 * @brief Pack argv/environment records into the process startup area.
 *
 * @param AC1 Process descriptor.
 * @param AC2 Packed-record source.
 * @param AC3 argc,,envc.
 * @param AC4 Four-word startup result.
 * @return AC1 zero.
 *
 * KCC spills this simple copy loop into a large local frame.  Keep the loop
 * state in AC10..AC16 and preserve those registers with one BLT pair instead.
 */
vm_space_startup:
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)

        move    10,2                   ; current source record
        move    12,4                   ; startup result
        hlrz    5,3                    ; argc
        hrrz    6,3                    ; envc
        hlrz    13,1(1)                ; logical user words
        subi    13,02000               ; startup-area logical base

        movem   5,(12)
        setz    7,
        jumpe   5,vm_startup_no_argv
        move    7,13
        addi    7,1
vm_startup_no_argv:
        movem   7,1(12)

        setz    7,
        jumpe   6,vm_startup_no_env
        move    7,13
        addi    7,1
        add     7,5
vm_startup_no_env:
        movem   7,2(12)

        hrrz    11,1(1)                ; physical user-space base
        add     11,13                  ; physical startup-area base
        movem   3,(11)                 ; argc,,envc metadata

        move    14,5
        add     14,6                   ; number of string records
        move    15,14
        addi    15,1                   ; skip metadata + pointer table
        jumpe   6,vm_startup_no_env_slot
        addi    15,1                   ; terminating environment pointer
vm_startup_no_env_slot:
        move    16,11
        addi    16,1                   ; next argv/env pointer slot

vm_startup_copy_loop:
        jumpe   14,vm_startup_copy_done
        move    1,(10)                 ; SIXBIT character count
        addi    1,5
        idivi   1,6
        addi    1,1                    ; count word + payload words
        push    17,1                   ; preserve nwords across copy

        move    2,13
        add     2,15
        movem   2,(16)                 ; logical string address
        move    2,11
        add     2,15                   ; physical string destination
        move    3,1
        move    1,10
        pushj   17,fs_copy_words

        pop     17,1
        add     10,1
        add     15,1
        addi    16,1
        sojg    14,vm_startup_copy_loop

vm_startup_copy_done:
        skipn   2(12)                  ; no envc -> no NULL terminator
        jrst    vm_startup_finish
        setzm   (16)
vm_startup_finish:
        move    1,13
        add     1,15
        subi    1,1
        movem   1,3(12)                ; initial user stack
        setz    1,

        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,kconst_7_7
        popj    17,

/**
 * @brief Move one inactive process extent and publish the new PDP-6 base.
 *
 * @param AC1 Owner/process slot.
 * @param AC2 Old physical base.
 * @param AC3 Extent words.
 * @param AC4 New aligned physical base.
 * @return AC1 MM_OK or MM_ERR_BUSY (-5).
 *
 * Keep the complete transaction in AC10..AC15 instead of KCC's spill frame.
 * The u-area STOP_MM bit excludes user execution while the physical image is
 * copied; the transition bit excludes scheduler/swap races.
 */
vm_extent_move:
        add     17,kconst_6_6
        movei   0,-5(17)
        hrli    0,10
        blt     0,(17)
        move    10,1                   ; owner
        move    11,2                   ; old base
        move    12,3                   ; words
        move    13,4                   ; new base

        camn    10,proc_current_slot
        jrst    vm_extent_move_busy
        move    14,10
        lsh     14,1
        add     14,10
        add     14,proc_table
        camn    13,11
        jrst    vm_extent_move_busy
        move    1,13
        trne    1,01777
        jrst    vm_extent_move_busy
        move    1,(14)
        trne    1,0200000              ; transition
        jrst    vm_extent_move_busy
        trnn    1,0400000              ; stable u-area present
        jrst    vm_extent_move_state
        hlrz    2,1
        move    2,024(2)
        trne    2,02                   ; translated user mapping held
        jrst    vm_extent_move_busy

vm_extent_move_state:
        move    15,2(14)
        lsh     15,-041                ; original process state
        move    1,(14)
        trnn    1,0400000
        jrst    vm_extent_move_nouarea

        hlrz    2,1
        move    3,024(2)
        movsi   4,01000                ; PROC_STOP_MM in control LH
        ior     3,4
        movem   3,024(2)
        move    1,10
        pushj   17,proc_runq_remove
        move    1,2(14)
        tlz     1,0700000
        tlo     1,0600000              ; PROC_STOP
        movem   1,2(14)
        jrst    vm_extent_move_mark

vm_extent_move_nouarea:
        caie    15,2                   ; resident SRUN without u-area cannot move
        jrst    vm_extent_move_mark
        jrst    vm_extent_move_busy

vm_extent_move_mark:
        movei   1,0200000
        iorm    1,(14)
        move    1,11
        move    2,13
        move    3,12
        pushj   17,fs_move_words
        hrrm    13,1(14)
        move    1,(14)
        andcmi  1,0200000
        movem   1,(14)

        trnn    1,0400000
        jrst    vm_extent_move_ok
        hlrz    2,1
        move    3,024(2)
        tlz     3,01000                ; clear only PROC_STOP_MM
        movem   3,024(2)
        tlne    3,01400                ; another stop reason remains
        jrst    vm_extent_move_ok
        move    1,2(14)
        tlz     1,0700000
        move    2,15
        andi    2,7
        lsh     2,041
        ior     1,2
        movem   1,2(14)
        caie    15,2
        jrst    vm_extent_move_ok
        move    1,10
        pushj   17,proc_runq_add

vm_extent_move_ok:
        setz    1,
        jrst    vm_extent_move_restore
vm_extent_move_busy:
        movni   1,5
vm_extent_move_restore:
        movei   0,10
        hrli    0,-5(17)
        blt     0,15
        sub     17,kconst_6_6
        popj    17,

/**
 * @brief Enter the first user process after KINIT; does not normally return.
 *
 * The bootstrap ABI supplies process, entry, stack and initial AC1-AC3. The
 * relocation base comes from backend-private vm_state RH.
 */
vm_enter_initial_user:
        move 5,-1(17)
        move 6,-2(17)
        movei 0,1(17)
        hrli 0,010
        blt 0,7(17)
        add 17,kconst_7_7
        pushj 17,vm_enter_initial_user_start
        movei 0,-6(17)
        hrl 0,0
        hrri 0,010
        blt 0,016
        sub 17,kconst_7_7
        popj 17,

vm_enter_initial_user_start:
        movem 17,mach_kernel_sp
        pushj 17,proc_record_kernel_sp

        ; APR DATAO: LH contains (user words - 02000), RH relocation base.
        move 0,3
        addi 0,1
        hrl 0,0
        hrrz 1,1(1)
        hrr 0,1
        movem 0,vm_pdp6_apr
        datao 0000,vm_pdp6_apr

        move 7,2
        setzi 16,0
        move 17,3
        move 1,4
        move 2,5
        move 3,6
        jrst 1,(7)

        .bss
vm_pdp6_apr:
        .word 0

        .text
/**
 * @brief Enter a committed EXEC replacement image; does not return.
 *
 * Preserve startup ACs, activate the new APR mapping, reset the stable private
 * kernel stack, and enter with the same argc/argv/envp ABI as initial RUN.
 */
        .globl proc_exec_enter
proc_exec_enter:
        move 7,1                    ; new entry
        move 6,2                    ; new user stack
        move 010,3                  ; argc
        move 011,4                  ; argv
        move 012,5                  ; envp
        pushj 17,vm_activate_current
        move 1,proc_current_ptr
        hlrz 5,(1)                  ; stable u-area base
        move 17,5
        addi 17,070                 ; PROC_USTACK_BASE
        movem 17,000021(5)
        movem 17,mach_kernel_sp
        setz 0,
        move 1,010
        move 2,011
        move 3,012
        move 17,6
        jrst 1,(7)
