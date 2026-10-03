; CRC -- CRC-16-CCITT OVER LOW EIGHT BITS OF BYTE INPUT.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        MOVEI   10,0177777
CRC_READ:
        SETZ    1,
        MOVE    2,CRC_BP
        MOVEI   3,01
        PUSHJ   17,U_READ_BYTES
        JUMPL   1,CRC_FAIL
        JUMPE   1,CRC_DONE
        MOVE    4,CRC_WORD
        LSH     4,-033
        ANDI    4,0377
        LSH     4,010
        XOR     10,04
        MOVEI   11,010
CRC_BIT:
        TRNE    10,0100000
        JRST    CRC_POLY
        LSH     10,01
        ANDI    10,0177777
        JRST    CRC_NEXT
CRC_POLY:
        LSH     10,01
        XORI    10,010041
        ANDI    10,0177777
CRC_NEXT:
        SOJG    11,CRC_BIT
        JRST    CRC_READ
CRC_DONE:
        MOVE    1,010
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        SETZ    1,
        PUSHJ   17,U_EXIT
CRC_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
.DATA
CRC_BP: .WORD POINT 9,CRC_WORD,8
CRC_WORD: .WORD 0
.INCLUDE "BASE.S"
