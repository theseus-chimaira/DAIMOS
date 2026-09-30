/**
 * @file pclk.s
 * @brief Resident PDP-6 Stanford Phil Petit calendar-clock reader.
 *
 * PCLK device 0730 supplies DAIMOS wall-clock UTC. It is independent of the
 * APR 60 Hz monotonic scheduler clock and is polled only when wall time is
 * requested. The routine is permanent KCORE code, has no BSS, and keeps no
 * state between calls.
 *
 * DATAI contains BCD year/month, zero-based day, hour, and minute plus the
 * historical 05004 offset. CONI contains minute/second information plus the
 * Petit/Panofsky 02020136700 offset. DATAI is sampled on both sides of CONI;
 * differing samples mean a minute boundary was crossed and the read retries.
 */

        .text
        .globl  pclk_time36
/**
 * @brief Read PCLK and return one coherent packed TIME36 value.
 * @return AC1 = TIME36 UTC value.
 *
 * AC1 holds the first DATAI sample and final result. AC2 holds CONI data;
 * AC3 holds the verification DATAI sample and then the BCD year; AC4 holds
 * the optional 21xx century bit. AC17 is only the normal return stack and is
 * otherwise untouched. No PDP-10-only instruction is required here.
 */
pclk_time36:
pclk_time36_retry:
        datai   0730,1
        coni    0730,2
        datai   0730,3
        came    1,3
        jrst    pclk_time36_retry

        subi    1,05004                 ; remove DATAI hardware offset
        move    3,1
        lsh     3,-024                  ; BCD year digits to low 8 bits
        andi    3,0377
        setz    4,
        cail    3,046                   ; 00..25 denote 2100..2125
        jrst    pclk_time36_century_done
        move    4,[0200000000000]
pclk_time36_century_done:
        subi    1,0600000               ; month code 4..17 -> month 1..12
        addi    1,04000                 ; zero-based day -> day 1..31
        lsh     1,6                     ; reserve low six bits for seconds
        sub     2,[02020136700]          ; remove CONI hardware offset
        lsh     2,-024
        andi    2,077
        ior     1,2
        ior     1,4
        popj    17,
