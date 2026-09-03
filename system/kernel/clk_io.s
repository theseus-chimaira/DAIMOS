; clk_io.s -- compact resident PDP-6 APR line-clock driver.
        .text
        .globl clk_pi_handler
        .globl clk_pi_service
        .globl clk_ticks
        .globl clk_tick_count
        .globl pdp10_pi_handler_return

; APR and the line clock share one PDP-6 PIA.  PI6 therefore still requires
; the clock flag qualification before this handler claims the interrupt.
clk_pi_handler:
        pushj 017,clk_pi_service
        jrst pdp10_pi_handler_return

; Callable PI6 service used by the DPY shared handler.
clk_pi_service:
        conso 0000,01000
        popj 017,
        aos clk_tick_count
        cono 0000,003006
        popj 017,

clk_ticks:
        move 1,clk_tick_count
        popj 017,

        .bss
clk_tick_count:
        .block 1
