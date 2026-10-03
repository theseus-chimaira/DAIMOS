; TR -- POSITIONAL CHARACTER TRANSLATION OF S6REC RECORDS.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,03
        JRST    TR_FAIL
        MOVEI   1,01
        PUSHJ   17,U_ARG
        MOVEM   1,TR_FROM
        MOVEI   1,02
        PUSHJ   17,U_ARG
        MOVEM   1,TR_TO
TR_READ:
        SETZ    1,
        PUSHJ   17,U_S6R_READ
        JUMPL   1,TR_FAIL
        JUMPE   1,TR_DONE
        MOVE    10,02
        SETZ    11,
TR_LINE:
        CAML    11,010
        JRST    TR_WRITE
        MOVE    1,U_REC_CHARS(011)
        PUSHJ   17,TR_MAP
        MOVEM   1,U_REC_CHARS(011)
        ADDI    11,01
        JRST    TR_LINE
TR_WRITE:
        MOVEI   1,01
        MOVE    2,010
        PUSHJ   17,U_S6R_WRITE
        JUMPL   1,TR_FAIL
        JRST    TR_READ
TR_DONE:
        SETZ    1,
        PUSHJ   17,U_EXIT
TR_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT

; AC1=ASCII CHAR, RETURN TRANSLATED CHAR.
TR_MAP:
        MOVE    12,01
        MOVE    2,TR_FROM
        MOVE    6,0(02)
        ANDI    6,0777777
        MOVE    5,U_S6BP0
        ADDI    2,01
        HRR     5,02
        SETZ    7,
TR_FIND:
        CAML    7,06
        JRST    TR_NOMATCH
        ILDB    3,05
        ADDI    3,040
        CAME    3,012
        JRST    TR_FIND_NEXT
        MOVE    2,TR_TO
        MOVE    4,0(02)
        ANDI    4,0777777
        CAMG    4,07
        JRST    TR_NOMATCH
        MOVE    5,U_S6BP0
        ADDI    2,01
        HRR     5,02
        MOVE    4,07
TR_SKIP:
        JUMPE   4,TR_GET
        IBP     05
        SOJG    4,TR_SKIP
TR_GET:
        ILDB    1,05
        ADDI    1,040
        POPJ    17,
TR_FIND_NEXT:
        ADDI    7,01
        JRST    TR_FIND
TR_NOMATCH:
        MOVE    1,012
        POPJ    17,
.DATA
TR_FROM: .WORD 0
TR_TO: .WORD 0
.INCLUDE "S6REC.S"
.INCLUDE "BASE.S"
