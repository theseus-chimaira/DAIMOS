; BASE.S -- SHARED DAIMOS PDP-6 USER UTILITY SUPPORT.
; ALL TEXT IS SIXBIT-SAFE SO THIS FILE CAN LIVE AS AN S6REC SOURCE.

.TEXT

U_EXIT:
        UUO     040,0(1)
        HALT

U_PUTC:
        PUSH    17,02
        ANDI    1,0777
        MOVE    2,01
        MOVEI   1,01
        UUO     062,0(1)
        POP     17,02
        POPJ    17,

U_PUTERR:
        PUSH    17,02
        ANDI    1,0777
        MOVE    2,01
        MOVEI   1,02
        UUO     062,0(1)
        POP     17,02
        POPJ    17,

U_SPACE:
        MOVEI   1,040
        PUSHJ   17,U_PUTC
        POPJ    17,

U_CRLF:
        MOVEI   1,015
        PUSHJ   17,U_PUTC
        MOVEI   1,012
        PUSHJ   17,U_PUTC
        POPJ    17,

; AC1 = COUNTED SIXBIT STRING POINTER.
U_PUTS6:
        MOVE    6,0(01)
        ANDI    6,0777777
        JUMPE   6,U_PUTS6_DONE
        MOVE    5,U_S6BP0
        ADDI    1,01
        HRR     5,01
U_PUTS6_LOOP:
        ILDB    1,05
        ADDI    1,040
        PUSHJ   17,U_PUTC
        SOJG    6,U_PUTS6_LOOP
U_PUTS6_DONE:
        POPJ    17,

; AC1 = ARGUMENT INDEX. RETURNS POINTER OR ZERO.
U_ARG:
        MOVE    3,U_ARGC
        SUB     3,01
        JUMPLE  3,U_ARG_NONE
        MOVE    2,U_ARGV
        ADD     2,01
        MOVE    1,0(02)
        POPJ    17,
U_ARG_NONE:
        SETZ    1,
        POPJ    17,

; AC1 = COUNTED SIXBIT STRING. RETURNS VALUE IN AC1, STATUS IN AC2.
U_PARSE_DEC:
        MOVE    6,0(01)
        ANDI    6,0777777
        JUMPE   6,U_PARSE_BAD
        MOVE    5,U_S6BP0
        ADDI    1,01
        HRR     5,01
        SETZ    7,
U_PARSE_DEC_LOOP:
        ILDB    4,05
        CAIGE   4,020
        JRST    U_PARSE_BAD
        CAILE   4,031
        JRST    U_PARSE_BAD
        IMULI   7,012
        SUBI    4,020
        ADD     7,04
        SOJG    6,U_PARSE_DEC_LOOP
        MOVE    1,07
        SETZ    2,
        POPJ    17,

U_PARSE_OCT:
        MOVE    6,0(01)
        ANDI    6,0777777
        JUMPE   6,U_PARSE_BAD
        MOVE    5,U_S6BP0
        ADDI    1,01
        HRR     5,01
        SETZ    7,
U_PARSE_OCT_LOOP:
        ILDB    4,05
        CAIGE   4,020
        JRST    U_PARSE_BAD
        CAILE   4,027
        JRST    U_PARSE_BAD
        LSH     7,03
        SUBI    4,020
        ADD     7,04
        SOJG    6,U_PARSE_OCT_LOOP
        MOVE    1,07
        SETZ    2,
        POPJ    17,
U_PARSE_BAD:
        SETZ    1,
        MOVEI   2,01
        POPJ    17,

; AC1 = UNSIGNED WORD. PRINT 12 OCTAL DIGITS.
U_PUTOCT12:
        MOVE    6,01
        MOVEI   5,014
        MOVEI   4,U_NUMBUF+013
U_PUTOCT12_FILL:
        MOVE    1,06
        ANDI    1,07
        ADDI    1,060
        MOVEM   1,0(04)
        LSH     6,-03
        SUBI    4,01
        SOJG    5,U_PUTOCT12_FILL
        MOVEI   6,014
        MOVEI   5,U_NUMBUF
U_PUTOCT12_OUT:
        MOVE    1,0(05)
        PUSHJ   17,U_PUTC
        ADDI    5,01
        SOJG    6,U_PUTOCT12_OUT
        POPJ    17,

; AC1 = UNSIGNED 18-BIT VALUE. PRINT 6 OCTAL DIGITS.
U_PUTOCT6:
        ANDI    1,0777777
        MOVE    6,01
        MOVEI   5,06
        MOVEI   4,U_NUMBUF+07
U_PUTOCT6_FILL:
        MOVE    1,06
        ANDI    1,07
        ADDI    1,060
        MOVEM   1,0(04)
        LSH     6,-03
        SUBI    4,01
        SOJG    5,U_PUTOCT6_FILL
        MOVEI   6,06
        MOVEI   5,U_NUMBUF+02
U_PUTOCT6_OUT:
        MOVE    1,0(05)
        PUSHJ   17,U_PUTC
        ADDI    5,01
        SOJG    6,U_PUTOCT6_OUT
        POPJ    17,

; AC1 = UNSIGNED WORD. PRINT DECIMAL WITHOUT LEADING ZEROES.
U_PUTDEC:
        PUSH    17,04
        PUSH    17,05
        PUSH    17,06
        PUSH    17,07
        MOVE    6,01
        MOVEI   4,U_NUMBUF+017
        SETZ    5,
        JUMPN   6,U_PUTDEC_FILL
        MOVEI   1,060
        PUSHJ   17,U_PUTC
        JRST    U_PUTDEC_DONE
U_PUTDEC_FILL:
        IDIVI   6,012
        ADDI    7,060
        MOVEM   7,0(04)
        SUBI    4,01
        ADDI    5,01
        JUMPN   6,U_PUTDEC_FILL
        ADDI    4,01
U_PUTDEC_OUT:
        MOVE    1,0(04)
        PUSHJ   17,U_PUTC
        ADDI    4,01
        SOJG    5,U_PUTDEC_OUT
U_PUTDEC_DONE:
        POP     17,07
        POP     17,06
        POP     17,05
        POP     17,04
        POPJ    17,

; AC1 = FILE DESCRIPTOR, AC2 = BUFFER, AC3 = WORD COUNT.
U_READ_WORDS:
        UUO     055,0(1)
        POPJ    17,

U_WRITE_WORDS:
        UUO     056,0(1)
        POPJ    17,

; AC1 = FILE DESCRIPTOR, AC2 = BYTE BUFFER, AC3 = BYTE COUNT.
U_READ_BYTES:
        PUSH    17,04
        PUSH    17,05
        PUSH    17,06
        MOVE    4,01
        MOVE    5,02
        MOVE    6,03
        SETZ    3,
U_READ_BYTES_LOOP:
        CAML    3,06
        JRST    U_READ_BYTES_DONE
        MOVE    1,04
        UUO     061,0(1)
        JUMPGE  1,U_READ_BYTES_CHAR
        CAME    1,[-1]
        JRST    U_READ_BYTES_DONE
        JRST    U_READ_BYTES_FAIL
U_READ_BYTES_CHAR:
        IDPB    1,05
        ADDI    3,01
        JRST    U_READ_BYTES_LOOP
U_READ_BYTES_DONE:
        MOVE    1,03
        JRST    U_READ_BYTES_RET
U_READ_BYTES_FAIL:
        SETO    1,
U_READ_BYTES_RET:
        POP     17,06
        POP     17,05
        POP     17,04
        POPJ    17,

U_WRITE_BYTES:
        PUSH    17,04
        PUSH    17,05
        PUSH    17,06
        MOVE    4,01
        MOVE    5,02
        MOVE    6,03
        SETZ    3,
U_WRITE_BYTES_LOOP:
        CAML    3,06
        JRST    U_WRITE_BYTES_DONE
        ILDB    2,05
        ANDI    2,0777
        MOVE    1,04
        UUO     062,0(1)
        JUMPL   1,U_WRITE_BYTES_FAIL
        ADDI    3,01
        JRST    U_WRITE_BYTES_LOOP
U_WRITE_BYTES_DONE:
        MOVE    1,03
        JRST    U_WRITE_BYTES_RET
U_WRITE_BYTES_FAIL:
        SETO    1,
U_WRITE_BYTES_RET:
        POP     17,06
        POP     17,05
        POP     17,04
        POPJ    17,

 ; RETURNS APPROXIMATE 60-HZ TICKS SINCE MIDNIGHT IN AC1 OR -1.
; CURRENT DAIMOS EXPOSES TIME36, NOT THE OLD RAW CLOCK-READ SERVICE.
U_TICKS:
        MOVEI   1,023
        UUO     077,0(1)
        JUMPL   1,U_TICKS_BAD
        MOVE    4,01
        MOVE    2,04
        LSH     2,-014
        ANDI    2,037
        IMULI   2,074
        IMULI   2,074
        MOVE    3,04
        LSH     3,-06
        ANDI    3,077
        IMULI   3,074
        ADD     2,03
        ANDI    4,077
        ADD     2,04
        IMULI   2,074
        MOVE    1,02
        POPJ    17,
U_TICKS_BAD:
        SETO    1,
        POPJ    17,

; PRINT STANDARD ERROR PREFIX. AC1 POINTS TO COUNTED SIXBIT NAME.
U_ERROR:
        PUSHJ   17,U_PUTS6
        MOVEI   1,072
        PUSHJ   17,U_PUTC
        PUSHJ   17,U_CRLF
        POPJ    17,

 .DATA
U_S6BP0:
        .WORD   POINT 6,0,-1
U_ARGC:
        .WORD   0
U_ARGV:
        .WORD   0

.BSS
U_NUMBUF:
        .BLOCK  020
