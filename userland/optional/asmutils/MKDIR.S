; MKDIR -- CREATE DIRECTORIES.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,01
        SETZ    11,
MKDIR_LOOP:
        MOVE    12,U_ARGC
        SUB     12,010
        JUMPLE  12,MKDIR_DONE
        MOVE    1,010
        PUSHJ   17,U_ARG
        MOVEI   2,0777
        UUO     051,0(1)
        JUMPGE  1,MKDIR_NEXT
        MOVEI   11,01
MKDIR_NEXT:
        ADDI    10,01
        JRST    MKDIR_LOOP
MKDIR_DONE:
        MOVE    1,011
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
