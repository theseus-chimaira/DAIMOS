; DIS -- NUMERIC PDP-10 INSTRUCTION FIELD DISASSEMBLER.
; OUTPUT: OFFSET WORD OPCODE AC I X Y, ALL IN OCTAL.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        SETZ    10,
        SETZ    11,
        CAILE   1,01
        JRST    DIS_OPEN
        JRST    DIS_LOOP
DIS_OPEN:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        SETZ    2,
        UUO     041,0(1)
        JUMPL   1,DIS_FAIL
        MOVE    10,01
DIS_LOOP:
        MOVE    1,010
        MOVEI   2,DIS_WORD
        MOVEI   3,01
        PUSHJ   17,U_READ_WORDS
        JUMPL   1,DIS_FAIL_CLOSE
        JUMPE   1,DIS_DONE
        MOVE    1,011
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        PUSHJ   17,U_PUTOCT12
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        LSH     1,-033
        ANDI    1,0777
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        LSH     1,-027
        ANDI    1,017
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        LSH     1,-026
        ANDI    1,01
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        LSH     1,-022
        ANDI    1,017
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_SPACE
        MOVE    1,DIS_WORD
        ANDI    1,0777777
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        ADDI    11,01
        JRST    DIS_LOOP
DIS_DONE:
        SKIPE   010
        PUSHJ   17,DIS_CLOSE
        SETZ    1,
        PUSHJ   17,U_EXIT
DIS_FAIL_CLOSE:
        SKIPE   010
        PUSHJ   17,DIS_CLOSE
DIS_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
DIS_CLOSE:
        MOVE    1,010
        UUO     042,0(1)
        POPJ    17,
.DATA
DIS_WORD: .WORD 0
.INCLUDE "BASE.S"
