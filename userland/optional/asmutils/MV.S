; MV -- RENAME ONE FILE OR DIRECTORY.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,03
        JRST    MV_FAIL
        MOVEI   1,01
        PUSHJ   17,U_ARG
        MOVE    10,01
        MOVEI   1,02
        PUSHJ   17,U_ARG
        MOVE    2,01
        MOVE    1,010
        UUO     053,0(1)
        JUMPL   1,MV_FAIL
        SETZ    1,
        PUSHJ   17,U_EXIT
MV_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
