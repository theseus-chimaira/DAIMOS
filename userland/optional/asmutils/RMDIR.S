; RMDIR -- REMOVE EMPTY DIRECTORIES.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,01
        SETZ    11,
RMDIR_LOOP:
        MOVE    12,U_ARGC
        SUB     12,010
        JUMPLE  12,RMDIR_DONE
        MOVE    1,010
        PUSHJ   17,U_ARG
        MOVE    2,01
        MOVEI   1,034
        UUO     077,0(1)
        JUMPGE  1,RMDIR_NEXT
        MOVEI   11,01
RMDIR_NEXT:
        ADDI    10,01
        JRST    RMDIR_LOOP
RMDIR_DONE:
        MOVE    1,011
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
