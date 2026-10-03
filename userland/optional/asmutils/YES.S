; YES -- REPEAT A STRING UNTIL INTERRUPTED.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAILE   1,01
        JRST    YES_ARG
        MOVEI   10,YES_DEFAULT
        JRST    YES_LOOP
YES_ARG:
        MOVEI   1,01
        PUSHJ   17,U_ARG
        MOVE    10,01
YES_LOOP:
        MOVE    1,010
        PUSHJ   17,U_PUTS6
        PUSHJ   17,U_CRLF
        JRST    YES_LOOP
.DATA
YES_DEFAULT:
        .WORD   01
        .SIXBIT /Y/
.INCLUDE "BASE.S"
