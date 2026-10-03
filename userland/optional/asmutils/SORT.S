; SORT -- BOUNDED LEXICAL SORT OF S6REC TEXT RECORDS.
; LIMIT: 32 RECORDS OF 120 CHARACTERS EACH.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        SETZ    10,
SORT_READ:
        SETZ    1,
        PUSHJ   17,U_S6R_READ
        JUMPL   1,SORT_FAIL
        JUMPE   1,SORT_BEGIN
        CAIGE   10,040
        JRST    SORT_STORE
        JRST    SORT_FAIL
SORT_STORE:
        MOVEM   2,SORT_LEN(010)
        MOVEM   10,SORT_INDEX(010)
        MOVE    11,010
        IMULI   11,0170
        ADDI    11,SORT_DATA
        SETZ    12,
SORT_COPY_IN:
        CAML    12,02
        JRST    SORT_NEXT_RECORD
        MOVE    3,U_REC_CHARS(012)
        MOVEM   3,0(011)
        ADDI    11,01
        ADDI    12,01
        JRST    SORT_COPY_IN
SORT_NEXT_RECORD:
        ADDI    10,01
        JRST    SORT_READ
SORT_BEGIN:
        MOVE    15,010
        CAILE   15,01
        JRST    SORT_OUTER_INIT
        JRST    SORT_OUTPUT
SORT_OUTER_INIT:
        SETZ    11,
SORT_OUTER:
        MOVE    12,015
        SUBI    12,01
        SUB     12,011
        JUMPLE  12,SORT_OUTPUT
        SETZ    13,
SORT_INNER:
        CAML    13,012
        JRST    SORT_OUTER_NEXT
        MOVE    1,SORT_INDEX(013)
        MOVE    2,SORT_INDEX+01(013)
        PUSHJ   17,SORT_COMPARE
        JUMPLE  1,SORT_NOSWAP
        MOVE    3,SORT_INDEX(013)
        MOVE    4,SORT_INDEX+01(013)
        MOVEM   4,SORT_INDEX(013)
        MOVEM   3,SORT_INDEX+01(013)
SORT_NOSWAP:
        ADDI    13,01
        JRST    SORT_INNER
SORT_OUTER_NEXT:
        ADDI    11,01
        JRST    SORT_OUTER
SORT_OUTPUT:
        SETZ    11,
SORT_OUTPUT_LOOP:
        CAML    11,015
        JRST    SORT_DONE
        MOVE    10,SORT_INDEX(011)
        MOVE    14,SORT_LEN(010)
        MOVE    12,010
        IMULI   12,0170
        ADDI    12,SORT_DATA
        SETZ    13,
SORT_COPY_OUT:
        CAML    13,014
        JRST    SORT_WRITE
        MOVE    3,0(012)
        MOVEM   3,U_REC_CHARS(013)
        ADDI    12,01
        ADDI    13,01
        JRST    SORT_COPY_OUT
SORT_WRITE:
        MOVEI   1,01
        MOVE    2,014
        PUSHJ   17,U_S6R_WRITE
        JUMPL   1,SORT_FAIL
        ADDI    11,01
        JRST    SORT_OUTPUT_LOOP
SORT_DONE:
        SETZ    1,
        PUSHJ   17,U_EXIT
SORT_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT

; AC1 AND AC2 ARE RECORD INDEXES. RETURN POSITIVE IF AC1 > AC2.
SORT_COMPARE:
        MOVE    10,01
        MOVE    14,02
        MOVE    6,SORT_LEN(010)
        MOVE    7,SORT_LEN(014)
        MOVE    3,010
        IMULI   3,0170
        ADDI    3,SORT_DATA
        MOVE    4,014
        IMULI   4,0170
        ADDI    4,SORT_DATA
        SETZ    5,
SORT_CMP_LOOP:
        CAML    5,06
        JRST    SORT_CMP_LEN
        CAML    5,07
        JRST    SORT_CMP_LEN
        MOVE    1,0(03)
        SUB     1,0(04)
        JUMPN   1,SORT_CMP_DONE
        ADDI    3,01
        ADDI    4,01
        ADDI    5,01
        JRST    SORT_CMP_LOOP
SORT_CMP_LEN:
        MOVE    1,06
        SUB     1,07
SORT_CMP_DONE:
        POPJ    17,
.DATA
SORT_COUNT: .WORD 0
.BSS
SORT_LEN: .BLOCK 040
SORT_INDEX: .BLOCK 040
SORT_DATA: .BLOCK 07400
.INCLUDE "S6REC.S"
.INCLUDE "BASE.S"
