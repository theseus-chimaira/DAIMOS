; PRIME -- TEST A POSITIVE 18-BIT DECIMAL INTEGER.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,02
        JRST    PRIME_ERROR
        MOVEI   1,01
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_DEC
        JUMPN   2,PRIME_ERROR
        CAILE   1,0777777
        JRST    PRIME_ERROR
        MOVE    10,01
        CAIGE   10,02
        JRST    PRIME_NO
        CAIE    10,02
        JRST    PRIME_ODD
        JRST    PRIME_YES
PRIME_ODD:
        TRNN    10,01
        JRST    PRIME_NO
        MOVEI   11,03
PRIME_LOOP:
        MOVE    6,011
        IMUL    6,011
        CAMLE   6,010
        JRST    PRIME_YES
        MOVE    6,010
        IDIV    6,011
        JUMPE   7,PRIME_NO
        ADDI    11,02
        JRST    PRIME_LOOP
PRIME_YES:
        MOVEI   1,PRIME_MSG
        PUSHJ   17,U_PUTS6
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
PRIME_NO:
        MOVEI   1,COMPOSITE_MSG
        PUSHJ   17,U_PUTS6
        PUSHJ   17,U_CRLF
        MOVEI   1,01
        PUSHJ   17,U_EXIT
PRIME_ERROR:
        MOVEI   1,02
        PUSHJ   17,U_EXIT
.DATA
PRIME_MSG: .WORD 05
        .SIXBIT /PRIME/
COMPOSITE_MSG: .WORD 011
        .SIXBIT /COMPOS/
        .SIXBIT /ITE/
.INCLUDE "BASE.S"
