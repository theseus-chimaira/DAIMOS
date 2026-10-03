; OD -- OCTAL DUMP OF EXACT 36-BIT WORDS.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        SETZ    10,
        SETZ    11,
        CAILE   1,01
        JRST    OD_OPEN
        JRST    OD_LOOP
OD_OPEN:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        SETZ    2,
        UUO     041,0(1)
        JUMPL   1,OD_FAIL
        MOVE    10,01
OD_LOOP:
        MOVE    1,010
        MOVEI   2,OD_WORD
        MOVEI   3,01
        PUSHJ   17,U_READ_WORDS
        JUMPL   1,OD_FAIL_CLOSE
        JUMPE   1,OD_DONE
        MOVE    1,011
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,OD_WORD
        PUSHJ   17,U_PUTOCT12
        PUSHJ   17,U_CRLF
        ADDI    11,01
        JRST    OD_LOOP
OD_DONE:
        SKIPE   010
        PUSHJ   17,OD_CLOSE
        SETZ    1,
        PUSHJ   17,U_EXIT
OD_FAIL_CLOSE:
        SKIPE   010
        PUSHJ   17,OD_CLOSE
OD_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
OD_CLOSE:
        MOVE    1,010
        UUO     042,0(1)
        POPJ    17,
.DATA
OD_WORD: .WORD 0
.INCLUDE "BASE.S"
