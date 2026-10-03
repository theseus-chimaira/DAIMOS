; ECHO -- PRINT ARGUMENTS SEPARATED BY SPACES.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,01
ECHO_LOOP:
        MOVE    6,U_ARGC
        SUB     6,010
        JUMPLE  6,ECHO_DONE
        MOVE    1,010
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PUTS6
        ADDI    10,01
        MOVE    6,U_ARGC
        SUB     6,010
        JUMPLE  6,ECHO_DONE
        PUSHJ   17,U_SPACE
        JRST    ECHO_LOOP
ECHO_DONE:
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
