/**
 * @file ka10_lowcore_v1968.s
 * @brief KA10 hardware workspace immediately after the fixed decoder.
 *
 * Appended only to the KCORE decoder object, never to Stage1. The
 * decoder ends at 0147; moving the existing APR shadow here aligns
 * the DF10 channel initial address without padding. These initialized
 * writable words are copied with KCORE before MINIT enables any I/O.
 * Two independent pairs are available. Each belongs to one channel
 * until acknowledged completion or verified quiescence. The link-time
 * map check enforces all five physical addresses. No runtime allocator.
 */
        .text
        .globl vm_pdp6_apr
        .globl ka10_df10_cia
        .globl ka10_df10_completion
vm_pdp6_apr:                  ; 0147, existing shadow, moved from BSS
        .word 0
ka10_df10_cia:                 ; 0150, even and below 01000
        .word 0
ka10_df10_completion:          ; 0151, terminator then hardware writeback
        .word 0
        .globl ka10_df10_cia1
        .globl ka10_df10_completion1
ka10_df10_cia1:                ; 0152, second independent channel
        .word 0
ka10_df10_completion1:         ; 0153, terminator then hardware writeback
        .word 0
