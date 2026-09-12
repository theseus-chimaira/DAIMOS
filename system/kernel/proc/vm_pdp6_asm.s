; vm_pdp6.s -- PDP-6 process-address-space backend.
;
; Generic kernel code treats struct proc word 1 as opaque VM state except for
; its LH logical-space size.  This backend stores the 02000-word-aligned
; physical relocation base in the private RH.

        .text
        .globl  vm_user_words
        .globl  vm_activate_current
        .globl  vm_enter_initial_user
        .globl  proc_current_slot
        .globl  proc_slot_ptr
        .globl  proc_record_kernel_sp
        .globl  mach_kernel_sp
        .globl  pdp10_ret_zero

; AC1 = logical user word address.
; Return AC1 = executive-accessible mapped address, or zero if invalid.
; AC3 = logical end of the contiguous mapping; AC4 = mapping bias, so callers
; needing the remaining span can compute AC4+AC3-AC1.  Keeping these outputs
; preserves the old PDP-6 hot-path instruction count while allowing a pager
; backend to provide a different executive mapping window.
vm_user_words:
        hrrz    1,1
        caige   1,020
        jrst    pdp10_ret_zero
        hlrz    3,vm_pdp6_apr
        addi    3,02000
        caml    1,3
        jrst    pdp10_ret_zero
        hrrz    4,vm_pdp6_apr
        add     1,4
        popj    17,

; Activate the current process address space for user return.
vm_activate_current:
        move    1,proc_current_slot
        pushj   17,proc_slot_ptr
        hlrz    2,1(1)
        subi    2,02000
        hrlz    2,2
        hrrz    3,1(1)
        hrr     2,3
        movem   2,vm_pdp6_apr
        datao   0000,vm_pdp6_apr
        popj    17,

; void vm_enter_initial_user(struct proc *p, entry, stack, ac1, ac2, ac3)
; The boot-only transition keeps the existing PDP-6 ABI, but obtains the
; relocation base from the backend-private RH of p->vm_state.
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
