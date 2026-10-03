; TOUCH -- CREATE FILES WITHOUT TRUNCATING EXISTING CONTENT.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,01
        SETZ    11,
TOUCH_LOOP:
        MOVE    12,U_ARGC
        SUB     12,010
        JUMPLE  12,TOUCH_DONE
        MOVE    1,010
        PUSHJ   17,U_ARG
        MOVEI   2,011
        UUO     041,0(1)
        JUMPL   1,TOUCH_BAD
        UUO     042,0(1)
        JUMPL   1,TOUCH_BAD
        JRST    TOUCH_NEXT
TOUCH_BAD:
        MOVEI   11,01
TOUCH_NEXT:
        ADDI    10,01
        JRST    TOUCH_LOOP
TOUCH_DONE:
        MOVE    1,011
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
