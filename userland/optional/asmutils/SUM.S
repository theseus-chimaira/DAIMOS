; SUM -- 36-BIT END-AROUND-FREE ADDITIVE WORD CHECKSUM.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        SETZ    10,
        SETZ    11,
        CAILE   1,01
        JRST    SUM_OPEN
        JRST    SUM_LOOP
SUM_OPEN:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        SETZ    2,
        UUO     041,0(1)
        JUMPL   1,SUM_FAIL
        MOVE    10,01
SUM_LOOP:
        MOVE    1,010
        MOVEI   2,SUM_WORD
        MOVEI   3,01
        PUSHJ   17,U_READ_WORDS
        JUMPL   1,SUM_FAIL_CLOSE
        JUMPE   1,SUM_DONE
        ADD     11,SUM_WORD
        JRST    SUM_LOOP
SUM_DONE:
        MOVE    1,011
        PUSHJ   17,U_PUTOCT12
        PUSHJ   17,U_CRLF
        SKIPE   010
        PUSHJ   17,SUM_CLOSE
        SETZ    1,
        PUSHJ   17,U_EXIT
SUM_FAIL_CLOSE:
        SKIPE   010
        PUSHJ   17,SUM_CLOSE
SUM_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
SUM_CLOSE:
        MOVE    1,010
        UUO     042,0(1)
        POPJ    17,
.DATA
SUM_WORD: .WORD 0
.INCLUDE "BASE.S"
