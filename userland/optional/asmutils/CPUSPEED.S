; CPUSPEED -- COUNT BATCHED INTEGER LOOPS DURING 60 CLOCK TICKS.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        PUSHJ   17,U_TICKS
        JUMPL   1,CPU_FAIL
        MOVE    10,01
        SETZ    11,
CPU_BATCH:
        MOVEI   12,01750
CPU_LOOP:
        ADDI    11,01
        SOJG    12,CPU_LOOP
        PUSHJ   17,U_TICKS
        JUMPL   1,CPU_FAIL
        SUB     1,010
        CAIGE   1,074
        JRST    CPU_BATCH
        MOVE    1,011
        PUSHJ   17,U_PUTDEC
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
CPU_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.INCLUDE "BASE.S"
