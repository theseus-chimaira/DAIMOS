; clk_io.s -- compact resident PDP-6 APR line-clock driver.
        .text
        .globl clk_pi_handler
        .globl clk_ticks
        .globl clk_tick_count
        .globl pdp10_pi_handler_return

; APR and the line clock share one PDP-6 PIA.  PI6 therefore still requires
; the clock flag qualification before this handler claims the interrupt.
clk_pi_handler:
        coni 0000,1
        trnn 1,01000
        jrst pdp10_pi_handler_return
        aos clk_tick_count
        movei 1,03006
        cono 0000,0(1)
        jrst pdp10_pi_handler_return

clk_ticks:
        move 1,clk_tick_count
        popj 17,

        .bss
clk_tick_count:
        .block 1
