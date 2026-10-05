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
        .globl  vm_enter_initial_user
        .globl  vm_space_startup
        .globl  vm_extent_move
        .globl  fs_copy_words
        .globl  fs_move_words
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
        move    0,0045(5)
        iori    0,02
        movem   0,0045(5)
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
        move    0,0045(5)
        andcmi  0,02
        movem   0,0045(5)
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
        move    2,045(2)
        trne    2,02                   ; translated user mapping held
        jrst    vm_extent_move_busy

vm_extent_move_state:
        move    15,2(14)
        lsh     15,-041                ; original process state
        move    1,(14)
        trnn    1,0400000
        jrst    vm_extent_move_nouarea

        hlrz    2,1
        move    3,045(2)
        movsi   4,01000                ; PROC_STOP_MM in control LH
        ior     3,4
        movem   3,045(2)
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
        move    3,045(2)
        tlz     3,01000                ; clear only PROC_STOP_MM
        movem   3,045(2)
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
        addi 17,0111                ; PROC_USTACK_BASE
        movem 17,000021(5)
        movem 17,mach_kernel_sp
        setz 0,
        move 1,010
        move 2,011
        move 3,012
        move 17,6
        jrst 1,(7)
