; kcore_io.s -- resident PDP-6 PI and CTY support used by KINIT and KCORE.
;
; The dispatcher/CTY machinery is the DAIMOSV1 KCORE implementation, reduced
; to the entries needed during V0.1 bring-up.  Unlike DAIMOSV1's fixed-image
; build, KINIT relocates KCORE to 060 first; kcore_pi_low_init then installs
; the fixed 020..057 PI state and vectors from resident KCORE code.

        .text
        .globl kcore_pi_low_init
        .globl pdp10_coni_cty
        .globl pdp10_cono_cty
        .globl pdp10_datai_cty
        .globl pdp10_datao_cty
        .globl pdp10_io_wait
        .globl pdp10_halt
        .globl pdp10_pi_hw_enable
        .globl pdp10_pi_hw_clear
        .globl pdp10_pi_dispatch
        .globl pdp10_pi_slots
        .globl pdp10_pi_level_span
        .globl pdp10_pi_count_words
        .globl pdp10_pi_unhandled_words
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

pdp10_coni_cty:
        coni 0120,pdp10_ioword
        move 1,pdp10_ioword
        popj 17,

pdp10_cono_cty:
        cono 0120,0(1)
        popj 17,

pdp10_datai_cty:
        datai 0120,pdp10_ioword
        move 1,pdp10_ioword
        popj 17,

pdp10_datao_cty:
        movem 1,pdp10_ioword
        datao 0120,pdp10_ioword
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

pdp10_halt:
        halt .
        jrst pdp10_halt

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
        aos pdp10_pi_count_words(10)
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
        aos pdp10_pi_unhandled_words(10)
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
        movem 2,000022
        movei 1,1
        movei 2,pdp10_pi_level1
        jrst pdp10_pi_common
pdp10_pi_level2:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,2
        movei 2,pdp10_pi_level2
        jrst pdp10_pi_common
pdp10_pi_level3:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,3
        movei 2,pdp10_pi_level3
        jrst pdp10_pi_common
pdp10_pi_level4:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,4
        movei 2,pdp10_pi_level4
        jrst pdp10_pi_common
pdp10_pi_level5:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,5
        movei 2,pdp10_pi_level5
        jrst pdp10_pi_common
pdp10_pi_level6:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,6
        movei 2,pdp10_pi_level6
        jrst pdp10_pi_common
pdp10_pi_level7:
        .word 0
        movem 1,000021
        movem 2,000022
        movei 1,7
        movei 2,pdp10_pi_level7
        jrst pdp10_pi_common

pdp10_pi_common:
        movem 0,000020
        move 0,[3,,000023]
        blt 0,000037
        move 3,pdp10_ioword
        movem 3,pdp10_pi_ioword
        move 3,0(2)
        movem 3,pdp10_pi_return_word
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
        move 0,[000023,,3]
        blt 0,17
        move 2,000022
        move 1,000021
        move 0,000020
        xct pdp10_pi_reenable
        jrst 10,@pdp10_pi_return_word

        .data
pdp10_ioword: .word 0
pdp10_pi_stack: .space 128
pdp10_pi_stack_end:
pdp10_pi_stack_limit_guard: .word 0
pdp10_pi_return_word: .word 0
pdp10_pi_ioword: .word 0
pdp10_pi_enabled_mask: .word 0
pdp10_pi_reenable: .word 0
