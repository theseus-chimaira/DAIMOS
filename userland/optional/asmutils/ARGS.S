.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        SETZ    10,
ARGS_LOOP:
        MOVE    6,U_ARGC
        SUB     6,010
        JUMPLE  6,ARGS_DONE
        MOVE    1,010
        PUSHJ   17,U_PUTDEC
        MOVEI   1,072
        PUSHJ   17,U_PUTC
        PUSHJ   17,U_SPACE
        MOVE    1,010
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PUTS6
        PUSHJ   17,U_CRLF
        ADDI    10,01
        JRST    ARGS_LOOP
ARGS_DONE:
        SETZ    1,
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
