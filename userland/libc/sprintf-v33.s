; DAIMOS sprintf entry for the KCC native PDP-10 varargs ABI.
;
; KCC passes the two named arguments in AC1 and AC2, with the unnamed
; arguments below the reconstructed named-argument slots on the stack.
; Form an exact native va_list and delegate to the one common formatter.
; This keeps the assembly entry small and avoids duplicating the integer,
; string, and floating-point format parser in processor-specific variants.
;
; PDP-6 instruction subset only: also usable on KA10 and later processors.
; No floating-point instructions are used in this entry.
        .text
        .globl sprintf
        .extern vsprintf
sprintf:
        pop     17,6
        add     17,[2,,2]
        movem   1,0(17)
        movem   2,-1(17)
        push    17,6
        add     17,[2,,2]
        movei   1,-4(17)
        hrli    1,0331100
        movem   1,0(17)
        move    2,0(17)
        move    3,-4(17)
        move    1,-3(17)
        move    6,3
        move    3,2
        move    2,6
        pushj   17,vsprintf
        sub     17,[2,,2]
        pop     17,6
        sub     17,[2,,2]
        jrst    0(6)
