; pclk_pdp10.s -- resident Stanford Petit calendar-clock reader.
;
; The Petit clock at device 0730 is the standard DAIMOS real-time clock.
; The APR 60 Hz clock remains the independent monotonic scheduler clock.
;
; DATAI contains BCD year/month, zero-based day, hour, and minute plus the
; historical 05004 offset.  CONI contains minute and second plus the
; Petit/Panofsky offset.  Read DATAI twice around CONI so a minute rollover
; cannot combine fields from two different minutes.
;
; Return the packed, chronologically sortable TIME36 value in AC1.  The
; supported century window is 2026..2125: BCD years 26..99 are 20xx and
; 00..25 are 21xx.

        .text
        .globl  pclk_time36
pclk_time36:
pclk_time36_retry:
        datai   0730,1
        coni    0730,2
        datai   0730,3
        camn    1,3
        jrst    pclk_time36_stable
        jrst    pclk_time36_retry

pclk_time36_stable:
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
