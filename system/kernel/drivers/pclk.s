/** Stanford Phil Petit PCLK coherent UTC reader, used by KINIT probe
 * and the optional PCLK MRES. Calendar decode preserves TIME36 semantics. */
        .text
        .globl pclk_time36
pclk_time36:
pclk_time36_retry:
        datai   0730,1
        ; The PDP-6 SIMH null device returns zero when PCLK is disabled.
        ; Do not decode that as a civil timestamp. TIME36 zero is unknown.
        jumpe   1,pclk_time36_done
        coni    0730,2
        datai   0730,3
        came    1,3
        jrst    pclk_time36_retry

        subi    1,05004                 ; remove DATAI hardware offset
        move    3,1
        lsh     3,-024                  ; BCD year digits to low 8 bits
        andi    3,0377
        subi    1,0600000               ; month code 4..17 -> month 1..12
        addi    1,04000                 ; zero-based day -> day 1..31
        lsh     1,6                     ; reserve low six bits for seconds
        sub     2,[02020136700]          ; remove CONI hardware offset
        lsh     2,-024
        andi    2,077
        ior     1,2
        ; The century bit is in the final TIME36 word; no temporary
        ; register or literal pool word is needed.
        caige   3,046                   ; 00..25 denote 2100..2125
        tlo     1,0200000
pclk_time36_done:
        popj    17,
