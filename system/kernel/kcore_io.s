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
; PI1..PI3 dispatch spans reuse low core 037, 040, and 041.  KINIT copies
; the Stage1 040/041 boot handoff into kcore_boot_handoff before MINIT runs,
; so these words are free for permanent PI state by module_pi_init().

        .text
        .globl pdp10_pi_handler_return
        .globl pdp10_pi_level1
        .globl pdp10_pi_level2
        .globl pdp10_pi_level3
        .globl pdp10_pi_level4
        .globl pdp10_pi_level5
        .globl pdp10_pi_level6
        .globl pdp10_pi_level7
        .globl pdp10_pi_handlers
        .globl pdp10_pi_level_span

pdp10_pi_level1:
        .word 0
        movem 1,000020
        movem 2,000021
        movem 3,000043
        move 2,000037
        movei 3,pdp10_pi_return_level1
        jrst pdp10_pi_dispatch
pdp10_pi_level2:
        .word 0
        movem 1,000022
        movem 2,000023
        movem 3,000045
        move 2,000040
        movei 3,pdp10_pi_return_level2
        jrst pdp10_pi_dispatch
pdp10_pi_level3:
        .word 0
        movem 1,000024
        movem 2,000025
        movem 3,000036
        move 2,000041
        movei 3,pdp10_pi_return_level3
        jrst pdp10_pi_dispatch
pdp10_pi_level4:
        .word 0
        movem 1,000026
        movem 2,000027
        movem 3,000051
        move 2,pdp10_pi_level_span+0
        movei 3,pdp10_pi_return_level4
        jrst pdp10_pi_dispatch
pdp10_pi_level5:
        .word 0
        movem 1,000030
        movem 2,000031
        movem 3,000053
        move 2,pdp10_pi_level_span+1
        movei 3,pdp10_pi_return_level5
        jrst pdp10_pi_dispatch
pdp10_pi_level6:
        .word 0
        movem 1,000032
        movem 2,000033
        movem 3,000055
        move 2,pdp10_pi_level_span+2
        movei 3,pdp10_pi_return_level6
        jrst pdp10_pi_dispatch
pdp10_pi_level7:
        .word 0
        movem 1,000034
        movem 2,000035
        movem 3,000057
        move 2,pdp10_pi_level_span+3
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
        move 1,000020(2)
        move 2,000021(2)
        jrst (3)

pdp10_pi_return_level1:
        move 3,000043
        jrst 010,@pdp10_pi_level1
pdp10_pi_return_level2:
        move 3,000045
        jrst 010,@pdp10_pi_level2
pdp10_pi_return_level3:
        move 3,000036
        jrst 010,@pdp10_pi_level3
pdp10_pi_return_level4:
        move 3,000051
        jrst 010,@pdp10_pi_level4
pdp10_pi_return_level5:
        move 3,000053
        jrst 010,@pdp10_pi_level5
pdp10_pi_return_level6:
        move 3,000055
        jrst 010,@pdp10_pi_level6
pdp10_pi_return_level7:
        move 3,000057
        jrst 010,@pdp10_pi_level7

        .bss
pdp10_pi_handlers:
        .block 010
pdp10_pi_level_span:
        .block 04
