; kcore_io.s -- minimal resident PDP-6 priority-interrupt runtime.
;
; Interrupt ABI:
;   AC1  handler scratch
;   AC2  compact handler span cursor
;   AC3  per-level return stub
;
; All other accumulators are untouched.  AC1..AC3 are saved separately for
; every PI level, so a higher-priority interrupt may safely nest at any point
; in the dispatcher, in a handler, or during the outer level's restore path.
;
; Low-core save cells:
;   level 1: 020,021,043       level 5: 030,031,053
;   level 2: 022,023,045       level 6: 032,033,055
;   level 3: 024,025,036       level 7: 034,035,057
;   level 4: 026,027,051
;
; The odd words of the seven two-word PI vectors are otherwise unused.
;
; PI dispatch spans live in resident storage.  Executive locations 040/041
; are reserved for the PDP-6 user UUO trap block.

        .text
        .globl pdp10_pi_handler_return
        .globl pdp10_pi_dispatch
        .globl pdp10_pi_dispatch_done
        .globl pdp10_pi_level1
        .globl pdp10_pi_level1_dispatch_jump
        .globl pdp10_pi_level2
        .globl pdp10_pi_level2_dispatch_jump
        .globl pdp10_pi_level3
        .globl pdp10_pi_level3_dispatch_jump
        .globl pdp10_pi_level4
        .globl pdp10_pi_level4_dispatch_jump
        .globl pdp10_pi_level5
        .globl pdp10_pi_level5_dispatch_jump
        .globl pdp10_pi_level6
        .globl pdp10_pi_level6_dispatch_jump
        .globl pdp10_pi_level7
        .globl pdp10_pi_handlers
        .globl pdp10_pi_level_span
        .globl pdp10_pi_sp_save
        .globl mach_kernel_sp

pdp10_pi_level1:
        .word 0
        movem 1,000020
        movem 2,000021
        movem 3,000043
        movem 17,pdp10_pi_sp_save+0
        move 1,pdp10_pi_level1
        tlnn 1,010000
        jrst pdp10_pi_level1_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level1_stack_ready:
        move 2,pdp10_pi_level_span+0
        movei 3,pdp10_pi_return_level1
pdp10_pi_level1_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level2:
        .word 0
        movem 1,000022
        movem 2,000023
        movem 3,000045
        movem 17,pdp10_pi_sp_save+2
        move 1,pdp10_pi_level2
        tlnn 1,010000
        jrst pdp10_pi_level2_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level2_stack_ready:
        move 2,pdp10_pi_level_span+1
        movei 3,pdp10_pi_return_level2
pdp10_pi_level2_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level3:
        .word 0
        movem 1,000024
        movem 2,000025
        movem 3,000036
        movem 17,pdp10_pi_sp_save+4
        move 1,pdp10_pi_level3
        tlnn 1,010000
        jrst pdp10_pi_level3_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level3_stack_ready:
        move 2,pdp10_pi_level_span+2
        movei 3,pdp10_pi_return_level3
pdp10_pi_level3_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level4:
        .word 0
        movem 1,000026
        movem 2,000027
        movem 3,000051
        movem 17,pdp10_pi_sp_save+6
        move 1,pdp10_pi_level4
        tlnn 1,010000
        jrst pdp10_pi_level4_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level4_stack_ready:
        move 2,pdp10_pi_level_span+3
        movei 3,pdp10_pi_return_level4
pdp10_pi_level4_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level5:
        .word 0
        movem 1,000030
        movem 2,000031
        movem 3,000053
        movem 17,pdp10_pi_sp_save+010
        move 1,pdp10_pi_level5
        tlnn 1,010000
        jrst pdp10_pi_level5_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level5_stack_ready:
        move 2,pdp10_pi_level_span+4
        movei 3,pdp10_pi_return_level5
pdp10_pi_level5_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level6:
        .word 0
        movem 1,000032
        movem 2,000033
        movem 3,000055
        movem 17,pdp10_pi_sp_save+012
        move 1,pdp10_pi_level6
        tlnn 1,010000
        jrst pdp10_pi_level6_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level6_stack_ready:
        move 2,pdp10_pi_level_span+5
        movei 3,pdp10_pi_return_level6
pdp10_pi_level6_dispatch_jump:
        jrst pdp10_pi_dispatch
pdp10_pi_level7:
        .word 0
        movem 1,000034
        movem 2,000035
        movem 3,000057
        movem 17,pdp10_pi_sp_save+014
        move 1,pdp10_pi_level7
        tlnn 1,010000
        jrst pdp10_pi_level7_stack_ready
        move 17,mach_kernel_sp
pdp10_pi_level7_stack_ready:
        move 2,pdp10_pi_level_span+6
        movei 3,pdp10_pi_return_level7

; A nonzero span is stored as -count,,start.  The first handler is called
; without modifying the cursor; AOBJN advances both count and slot index.
pdp10_pi_dispatch:
        jumpe 2,pdp10_pi_dispatch_done
        jrst @pdp10_pi_handlers(2)

; Every resident interrupt handler returns here and preserves AC2/AC3.
pdp10_pi_handler_return:
        aobjn 2,pdp10_pi_dispatch
pdp10_pi_dispatch_done:
pdp10_pi_return_common:

; AC3 still names the level-specific return stub.  The stubs are two words
; apart, exactly matching the two-word stride of the AC1/AC2 save pairs in
; low core.  Derive that offset once, restore AC1/AC2 here, then let the
; original per-level tail restore AC3 and dismiss the correct PI level.
        move 2,3
        subi 2,pdp10_pi_return_level1
        move 17,pdp10_pi_sp_save(2)
        move 1,000020(2)
        move 2,000021(2)
        jrst (3)

pdp10_pi_return_level1:
        move 3,000043
        jrst 012,@pdp10_pi_level1
pdp10_pi_return_level2:
        move 3,000045
        jrst 012,@pdp10_pi_level2
pdp10_pi_return_level3:
        move 3,000036
        jrst 012,@pdp10_pi_level3
pdp10_pi_return_level4:
        move 3,000051
        jrst 012,@pdp10_pi_level4
pdp10_pi_return_level5:
        move 3,000053
        jrst 012,@pdp10_pi_level5
pdp10_pi_return_level6:
        move 3,000055
        jrst 012,@pdp10_pi_level6
pdp10_pi_return_level7:
        move 3,000057
        jrst 012,@pdp10_pi_level7

        .bss
pdp10_pi_handlers:
        .block 015
pdp10_pi_level_span:
        .block 07
pdp10_pi_sp_save:
        .block 1                       ; PI1 stack slot, offset 0
        .globl proc_table
proc_table:
        .block 1                       ; unreachable PI offset 1
        .block 1                       ; PI2 stack slot, offset 2
        .globl proc_slots
proc_slots:
        .block 1                       ; unreachable PI offset 3
        .block 1                       ; PI3 stack slot, offset 4
        .globl proc_high_slot
proc_high_slot:
        .block 1                       ; unreachable PI offset 5
        .block 1                       ; PI4 stack slot, offset 6
        .globl proc_current_slot
proc_current_slot:
        .block 1                       ; unreachable PI offset 7
        .block 1                       ; PI5 stack slot, offset 010
        .globl proc_sched_cursor
proc_sched_cursor:
        .block 1                       ; unreachable PI offset 011
        .block 1                       ; PI6 stack slot, offset 012
        .globl proc_sched_kick
proc_sched_kick:
        .block 1                       ; unreachable PI offset 013
        .block 1                       ; PI7 stack slot, offset 014
        .globl proc_sched_deferred_ticks
proc_sched_deferred_ticks:
        .block 1
        .globl proc_runq_head
proc_runq_head:
        .block 1

        .text
; Save the PI state and suppress new priority interrupts while MM publishes a
; relocated module.  The caller restores only the global on/off state; level
; enables remain untouched by CONO PI,0400/0200.
        .globl mach_pi_disable
        .globl mach_pi_restore
mach_pi_disable:
        coni 0004,1
        cono 0004,000400
        popj 017,
mach_pi_restore:
        trnn 1,000200
        popj 017,
        cono 0004,000200
        popj 017,
