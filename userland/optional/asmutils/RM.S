; RM -- UNLINK FILES. -F IGNORES MISSING FILES.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,01
        SETZ    11,
        SETZ    12,
        CAILE   1,01
        PUSHJ   17,RM_OPTION
RM_LOOP:
        MOVE    13,U_ARGC
        SUB     13,010
        JUMPLE  13,RM_DONE
        MOVE    1,010
        PUSHJ   17,U_ARG
        UUO     052,0(1)
        JUMPGE  1,RM_NEXT
        SKIPN   012
        MOVEI   11,01
RM_NEXT:
        ADDI    10,01
        JRST    RM_LOOP
RM_DONE:
        MOVE    1,011
        PUSHJ   17,U_EXIT
RM_OPTION:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        MOVE    3,0(01)
        ANDI    3,0777777
        CAIE    3,02
        POPJ    17,
        MOVE    3,01(01)
        CAME    3,RM_FWORD
        POPJ    17,
        MOVEI   10,02
        MOVEI   12,01
        POPJ    17,
.DATA
RM_FWORD:
        .SIXBIT /-F/
.INCLUDE "BASE.S"
