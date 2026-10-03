; REV -- REVERSE EACH S6REC TEXT RECORD.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
REV_READ:
        SETZ    1,
        PUSHJ   17,U_S6R_READ
        JUMPL   1,REV_FAIL
        JUMPE   1,REV_DONE
        MOVE    10,02
        SETZ    11,
        MOVE    12,02
        SUBI    12,01
REV_SWAP:
        CAML    11,012
        JRST    REV_WRITE
        MOVE    3,U_REC_CHARS(011)
        MOVE    4,U_REC_CHARS(012)
        MOVEM   4,U_REC_CHARS(011)
        MOVEM   3,U_REC_CHARS(012)
        ADDI    11,01
        SUBI    12,01
        JRST    REV_SWAP
REV_WRITE:
        MOVEI   1,01
        MOVE    2,010
        PUSHJ   17,U_S6R_WRITE
        JUMPL   1,REV_FAIL
        JRST    REV_READ
REV_DONE:
        SETZ    1,
        PUSHJ   17,U_EXIT
REV_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "S6REC.S"
.INCLUDE "BASE.S"
