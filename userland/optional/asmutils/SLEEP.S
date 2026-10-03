; SLEEP -- MONOTONIC SLEEP FOR A DECIMAL NUMBER OF SECONDS.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,02
        JRST    SLEEP_FAIL
        MOVEI   1,01
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_DEC
        JUMPN   2,SLEEP_FAIL
        IMULI   1,074
        MOVE    2,01
        MOVEI   1,036
        UUO     077,0(1)
        JUMPL   1,SLEEP_FAIL
        SETZ    1,
        PUSHJ   17,U_EXIT
SLEEP_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
