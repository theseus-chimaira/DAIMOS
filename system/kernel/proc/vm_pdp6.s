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
        .globl  proc_current_slot
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
        move    5,proc_current_slot
        lsh     5,1
        add     5,proc_current_slot
        add     5,proc_table
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
        move    5,proc_current_slot
        lsh     5,1
        add     5,proc_current_slot
        add     5,proc_table
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
        move    1,proc_current_slot
        pushj   17,proc_slot_ptr
        move    2,1(1)                  ; user words,,physical base
        sub     2,[02000,,0]            ; APR LH stores words-02000
        movem   2,vm_pdp6_apr
        datao   0000,vm_pdp6_apr
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
        add 17,[7,,7]
        pushj 17,vm_enter_initial_user_start
        movei 0,-6(17)
        hrl 0,0
        hrri 0,010
        blt 0,016
        sub 17,[7,,7]
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
        move 1,proc_current_slot
        pushj 17,proc_slot_ptr
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
