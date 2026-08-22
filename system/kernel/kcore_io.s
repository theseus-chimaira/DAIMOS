; kcore_io.s -- resident PDP-6 priority-interrupt core support.
;

        .text
        .globl kcore_pi_low_init
        .globl pdp10_io_wait
        .globl pdp10_pi_hw_enable
        .globl pdp10_pi_hw_clear
        .globl pdp10_pi_dispatch
        .globl pdp10_pi_slots
        .globl pdp10_pi_level_span
        .globl mach_words_zero
        .globl mach_pi_stack_prepare

; Install DAIMOSV1's fixed PDP-6 PI low-core layout after KINIT has saved
; 040/041.  020..037 are AC save words; 040/041 are PI-private state; the
; seven two-word vectors occupy 042..057.
kcore_pi_low_init:
        setzm 000020
        move 1,[000020,,000021]
        blt 1,000037
        move 1,[012345670123]
        movem 1,000040
        setzm 000041
        move 1,[jsr pdp10_pi_level1]
        movem 1,000042
        setzm 000043
        move 1,[jsr pdp10_pi_level2]
        movem 1,000044
        setzm 000045
        move 1,[jsr pdp10_pi_level3]
        movem 1,000046
        setzm 000047
        move 1,[jsr pdp10_pi_level4]
        movem 1,000050
        setzm 000051
        move 1,[jsr pdp10_pi_level5]
        movem 1,000052
        setzm 000053
        move 1,[jsr pdp10_pi_level6]
        movem 1,000054
        setzm 000055
        move 1,[jsr pdp10_pi_level7]
        movem 1,000056
        setzm 000057
        popj 17,

pdp10_io_wait:
        move 2,1
        jumple 2,pdp10_wait_done
pdp10_wait_loop:
        sojg 2,pdp10_wait_loop
pdp10_wait_done:
        popj 17,

mach_words_zero:
        jumpe 2,mach_words_zero_ret
        setzm 0(1)
        soje 2,mach_words_zero_ret
        move 3,1
        addi 3,1
        hrl 3,1
        move 4,1
        add 4,2
        blt 3,0(4)
mach_words_zero_ret:
        popj 17,

mach_pi_stack_prepare:
        move 1,000040
        movem 1,pdp10_pi_stack
        movei 2,pdp10_pi_stack+1
        hrli 2,pdp10_pi_stack
        blt 2,pdp10_pi_stack_end-1
        movem 1,pdp10_pi_stack_limit_guard
        setzm 000041
        popj 17,

pdp10_pi_hw_enable:
        andi 1,0177
        iorm 1,pdp10_pi_enabled_mask
        move 1,pdp10_pi_enabled_mask
        iori 1,002200
        move 2,[cono 0004,0]
        hrr 2,1
        movem 2,pdp10_pi_reenable
        cono 0004,0(1)
        popj 17,

pdp10_pi_hw_clear:
        setzm pdp10_pi_enabled_mask
        cono 0004,010000
        popj 17,

; Native dispatcher for the compact handler span registered at one PI level.
pdp10_pi_dispatch:
        cail 1,1
        caile 1,7
        jrst pdp10_pi_dispatch_bad
        move 10,1
        hrrz 11,pdp10_pi_level_span(10)
        hlrz 12,pdp10_pi_level_span(10)
        lsh 11,1
        setz 13,
        jumpe 12,pdp10_pi_dispatch_done
pdp10_pi_dispatch_loop:
        move 4,pdp10_pi_slots(11)
        move 1,10
        move 2,pdp10_pi_slots+1(11)
        pushj 17,(4)
        cain 1,1
        jrst pdp10_pi_dispatch_next
        jumpn 1,pdp10_pi_dispatch_ret
        movei 13,1
pdp10_pi_dispatch_next:
        addi 11,2
        sojg 12,pdp10_pi_dispatch_loop
pdp10_pi_dispatch_done:
        jumpn 13,pdp10_pi_dispatch_ok
pdp10_pi_dispatch_ok:
        setz 1,
pdp10_pi_dispatch_ret:
        popj 17,
pdp10_pi_dispatch_bad:
        seto 1,
        popj 17,

pdp10_pi_level1:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+0
        movei 1,1
        movei 2,pdp10_pi_level1
        jrst pdp10_pi_common
pdp10_pi_level2:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+1
        movei 1,2
        movei 2,pdp10_pi_level2
        jrst pdp10_pi_common
pdp10_pi_level3:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+2
        movei 1,3
        movei 2,pdp10_pi_level3
        jrst pdp10_pi_common
pdp10_pi_level4:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+3
        movei 1,4
        movei 2,pdp10_pi_level4
        jrst pdp10_pi_common
pdp10_pi_level5:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+4
        movei 1,5
        movei 2,pdp10_pi_level5
        jrst pdp10_pi_common
pdp10_pi_level6:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+5
        movei 1,6
        movei 2,pdp10_pi_level6
        jrst pdp10_pi_common
pdp10_pi_level7:
        .word 0
        movem 1,000021
        movem 2,pdp10_pi_ac2_save+6
        movei 1,7
        movei 2,pdp10_pi_level7
        jrst pdp10_pi_common

pdp10_pi_common:
        movem 0,000020
        move 0,[3,,000023]
        blt 0,000037
        move 3,pdp10_ioword
        movem 3,pdp10_pi_ioword
        cono 0004,000400
        movei 17,pdp10_pi_stack-1
        setz 16,
        pushj 17,pdp10_pi_dispatch
        move 3,pdp10_pi_stack_limit_guard
        camn 3,000040
        jrst pdp10_pi_stack_ok
        halt .
pdp10_pi_stack_ok:
        move 3,pdp10_pi_ioword
        movem 3,pdp10_ioword
        move 2,pdp10_pi_epilogue_table-1(10)
        move 0,[000023,,3]
        blt 0,17
        move 1,000021
        move 0,000020
        jrst 0(2)

pdp10_pi_return_level1:
        move 2,pdp10_pi_ac2_save+0
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level1
pdp10_pi_return_level2:
        move 2,pdp10_pi_ac2_save+1
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level2
pdp10_pi_return_level3:
        move 2,pdp10_pi_ac2_save+2
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level3
pdp10_pi_return_level4:
        move 2,pdp10_pi_ac2_save+3
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level4
pdp10_pi_return_level5:
        move 2,pdp10_pi_ac2_save+4
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level5
pdp10_pi_return_level6:
        move 2,pdp10_pi_ac2_save+5
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level6
pdp10_pi_return_level7:
        move 2,pdp10_pi_ac2_save+6
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_level7

        .data
pdp10_pi_epilogue_table:
        .word pdp10_pi_return_level1
        .word pdp10_pi_return_level2
        .word pdp10_pi_return_level3
        .word pdp10_pi_return_level4
        .word pdp10_pi_return_level5
        .word pdp10_pi_return_level6
        .word pdp10_pi_return_level7
pdp10_ioword: .word 0
pdp10_pi_stack: .space 128
pdp10_pi_stack_end:
pdp10_pi_stack_limit_guard: .word 0
pdp10_pi_ac2_save: .space 7
pdp10_pi_ioword: .word 0
pdp10_pi_enabled_mask: .word 0
pdp10_pi_reenable: .word 0
