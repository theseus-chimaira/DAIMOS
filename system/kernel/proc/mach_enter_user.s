; mach_enter_user.s -- boot-only PDP-6 initial protected-user transition.
;
; User addresses remain logical.  DATAO APR supplies the aligned physical
; relocation base and logical protection limit; JRST 1 enters user mode.

        .text
        .globl mach_enter_user
        .globl mach_kernel_sp
        .globl mach_user_apr
        .globl proc_record_kernel_sp

; void mach_enter_user(base, entry, stack, ac1, ac2, ac3)
; base is a physical 02000-word-aligned MM extent base.  entry and stack are
; logical addresses.  stack is alloc_words - EXEC_DXR_STACK_WORDS - 1, so
; stack+1 is exactly the protection-register value for a 02000-word stack.
mach_enter_user:
        move 5,-1(17)
        move 6,-2(17)
        movei 0,1(17)
        hrli 0,010
        blt 0,7(17)
        add 17,[7,,7]
        pushj 17,mach_enter_user_start
        movei 0,-6(17)
        hrl 0,0
        hrri 0,010
        blt 0,016
        sub 17,[7,,7]
        popj 17,

mach_enter_user_start:
        movem 17,mach_kernel_sp
        pushj 17,proc_record_kernel_sp

        ; APR DATAO: RH high address bits -> relocation register; LH high
        ; address bits -> protection register.  The hardware adds 01777 to
        ; the latter, so stack+1 (= alloc_words-02000) is the correct value.
        move 0,3
        addi 0,1
        hrl 0,0
        hrr 0,1
        movem 0,mach_user_apr
        datao 0000,mach_user_apr

        move 7,2
        setzi 16,0
        move 17,3
        move 1,4
        move 2,5
        move 3,6
        jrst 1,(7)

        .bss
mach_user_apr:
        .word 0
