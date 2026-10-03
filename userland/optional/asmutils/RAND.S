; RAND -- 36-BIT LINEAR CONGRUENTIAL RANDOM WORD GENERATOR.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        PUSHJ   17,U_TICKS
        JUMPL   1,RAND_SEED_ONE
        MOVE    10,01
        JRST    RAND_ARGS
RAND_SEED_ONE:
        MOVEI   10,01
RAND_ARGS:
        MOVEI   11,01
        MOVE    12,U_ARGC
        CAILE   12,01
        JRST    RAND_GET_SEED
        JRST    RAND_RUN
RAND_GET_SEED:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_OCT
        JUMPN   2,RAND_FAIL
        MOVE    10,01
        MOVE    12,U_ARGC
        CAILE   12,02
        JRST    RAND_GET_COUNT
        JRST    RAND_RUN
RAND_GET_COUNT:
        MOVEI   1,02
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_DEC
        JUMPN   2,RAND_FAIL
        MOVE    11,01
RAND_RUN:
        JUMPE   11,RAND_DONE
RAND_LOOP:
        IMULI   10,040715
        ADDI    10,01
        MOVE    1,010
        PUSHJ   17,U_PUTOCT12
        PUSHJ   17,U_CRLF
        SOJG    11,RAND_LOOP
RAND_DONE:
        SETZ    1,
        PUSHJ   17,U_EXIT
RAND_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
