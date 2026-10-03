; FACTOR -- FACTOR A POSITIVE 18-BIT DECIMAL INTEGER.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,02
        JRST    FACT_FAIL
        MOVEI   1,01
        PUSHJ   17,U_ARG
        PUSHJ   17,U_PARSE_DEC
        JUMPN   2,FACT_FAIL
        CAILE   1,0777777
        JRST    FACT_FAIL
        MOVE    10,01
        CAIGE   10,02
        JRST    FACT_SINGLE
        MOVEI   11,02
        SETZ    12,
FACT_TRY:
        MOVE    6,010
        IDIV    6,011
        JUMPE   7,FACT_FOUND
        ADDI    11,01
        MOVE    6,011
        IMUL    6,011
        CAMLE   6,010
        JRST    FACT_LAST
        JRST    FACT_TRY
FACT_FOUND:
        SKIPN   012
        JRST    FACT_PRINT
        PUSHJ   17,U_SPACE
FACT_PRINT:
        MOVE    1,011
        PUSHJ   17,U_PUTDEC
        MOVEI   12,01
        MOVE    10,06
        CAILE   10,01
        JRST    FACT_TRY
        JRST    FACT_DONE
FACT_LAST:
        SKIPN   012
        JRST    FACT_LAST_PRINT
        PUSHJ   17,U_SPACE
FACT_LAST_PRINT:
        MOVE    1,010
        PUSHJ   17,U_PUTDEC
        JRST    FACT_DONE
FACT_SINGLE:
        MOVE    1,010
        PUSHJ   17,U_PUTDEC
FACT_DONE:
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
FACT_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
