; BENCH -- TIME A FIXED INTEGER ADD LOOP.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,0303240
        CAILE   1,01
        JRST    BENCH_ARG
        JRST    BENCH_RUN
BENCH_ARG:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_DEC
        JUMPN   2,BENCH_FAIL
        MOVE    10,01
BENCH_RUN:
        PUSHJ   17,U_TICKS
        JUMPL   1,BENCH_FAIL
        MOVE    11,01
        MOVE    12,010
        SETZ    13,
BENCH_LOOP:
        ADDI    13,01
        SOJG    12,BENCH_LOOP
        PUSHJ   17,U_TICKS
        JUMPL   1,BENCH_FAIL
        SUB     1,011
        MOVE    14,01
        MOVE    1,010
        PUSHJ   17,U_PUTDEC
        PUSHJ   17,U_SPACE
        MOVE    1,014
        PUSHJ   17,U_PUTDEC
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
BENCH_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
