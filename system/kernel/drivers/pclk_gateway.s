/** Resident wall-clock gateway. KINIT binds one provider to its JRST. */
        .text
        .globl wall_time36
        .globl wall_time36_provider_jump
        .globl wall_clock_set
        .globl wall_clock_set_provider_jump
        .globl wall_clock_ticks
        .globl clk_tick_count
wall_time36:
wall_time36_provider_jump:
        jrst wall_time36_unset
wall_time36_unset:
        setz 1,
        popj 17,
wall_clock_set:
wall_clock_set_provider_jump:
        jrst wall_clock_set_unavailable
wall_clock_set_unavailable:
        seto 1,
        popj 17,
        .bss
; Shared APR and software calendar tick word; zeroed at boot.
wall_clock_ticks:
clk_tick_count:
        .block 1
