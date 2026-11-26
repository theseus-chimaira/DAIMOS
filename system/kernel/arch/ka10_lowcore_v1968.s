/**
 * @file ka10_lowcore_v1968.s
 * @brief KA10 hardware workspace immediately after the fixed decoder.
 *
 * Appended only to the KCORE decoder object, never to Stage1. The
 * decoder ends at 0147; moving the existing APR shadow here aligns
 * the DF10 channel initial address without padding. These initialized
 * writable words are copied with KCORE before MINIT enables any I/O.
 * The pair belongs to one channel at a time; it must not be used by
 * another controller until the former channel has been quiesced.
 * The link-time map check enforces all three physical addresses.
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
