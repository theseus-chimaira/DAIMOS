; clk_io.s -- compact resident PDP-6 APR line-clock driver.
        .text
        .globl clk_pi_handler
        .globl clk_pi_service
        .globl clk_ticks
        .globl clk_tick_count
        .globl pdp10_pi_handler_return
        .globl proc_sched_pi_tick
        .globl proc_sched_pi_resched
        .globl proc_sched_kick

; APR and the line clock share one PDP-6 PIA.  PI6 therefore still requires
; the clock flag qualification before this handler claims the interrupt.
clk_pi_handler:
        pushj 017,clk_pi_service
        jrst pdp10_pi_handler_return

; Callable PI6 service used by the DPY shared handler.
clk_pi_service:
        ; PI handlers must preserve AC2/AC3: the common dispatcher keeps its
        ; handler-span cursor and level return stub there.  Scheduler entry
        ; uses both registers, so protect them around the shared clock service.
        push 017,2
        push 017,3
        conso 0000,01000
        jrst clk_pi_kick
        aos clk_tick_count
        cono 0000,003006
        setzm proc_sched_kick
        pushj 017,proc_sched_pi_tick
        jrst clk_pi_service_done

; A blocking kernel path can request PI6 in software.  This is distinct from
; a timer tick: it switches away immediately but does not charge CPU time or
; advance sleep age.  If a real clock tick coincides with the request, the
; normal tick path above satisfies both with a single scheduling decision.
clk_pi_kick:
        skipn proc_sched_kick
        jrst clk_pi_service_done
        setzm proc_sched_kick
        pushj 017,proc_sched_pi_resched
clk_pi_service_done:
        pop 017,3
        pop 017,2
        popj 017,

clk_ticks:
        move 1,clk_tick_count
        popj 017,

        .bss
clk_tick_count:
        .block 1
