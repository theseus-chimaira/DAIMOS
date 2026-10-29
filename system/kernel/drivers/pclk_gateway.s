/** Resident fallback routines. KINIT binds known consumers directly. */
        .text
        .globl wall_time36
        .globl wall_clock_set
        .globl wall_clock_ticks
        .globl clk_tick_count
wall_time36:
wall_time36_unset:
        setz 1,
        popj 17,
wall_clock_set:
wall_clock_set_unavailable:
        seto 1,
        popj 17,
        .bss
; Shared APR and software calendar tick word; zeroed at boot.
wall_clock_ticks:
clk_tick_count:
        .block 1
