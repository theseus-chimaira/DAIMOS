; S6REC.S -- BOUNDED S6REC TEXT INPUT AND OUTPUT.
; MAXIMUM RECORD LENGTH IS 120 SIXBIT CHARACTERS.

.TEXT

; AC1 = FD. RETURNS AC1=1 RECORD, 0 EOF, -1 ERROR; AC2=LENGTH.
U_S6R_READ:
        MOVEM   1,U_REC_FD
        MOVEI   2,U_REC_HEADER
        MOVEI   3,01
        PUSHJ   17,U_READ_WORDS
        JUMPL   1,U_S6R_ERROR
        JUMPE   1,U_S6R_EOF
        CAIE    1,01
        JRST    U_S6R_ERROR
        MOVE    6,U_REC_HEADER
        MOVE    7,06
        LSH     7,-036
        ANDI    7,077
        CAIE    7,01
        JRST    U_S6R_ERROR
        ANDI    6,077777777
        CAILE   6,0170
        JRST    U_S6R_ERROR
        MOVEM   6,U_REC_LEN
        MOVE    5,U_REC_LEN
        ADDI    5,05
        IDIVI   5,06
        MOVEM   5,U_REC_NWORDS
        JUMPE   5,U_S6R_UNPACK
        MOVE    1,U_REC_FD
        MOVEI   2,U_REC_WORDS
        MOVE    3,05
        PUSHJ   17,U_READ_WORDS
        JUMPL   1,U_S6R_ERROR
        CAME    1,U_REC_NWORDS
        JRST    U_S6R_ERROR
U_S6R_UNPACK:
        MOVE    6,U_REC_LEN
        JUMPE   6,U_S6R_OK
        MOVE    5,U_S6BP0
        MOVEI   4,U_REC_WORDS
        HRR     5,04
        MOVEI   4,U_REC_CHARS
U_S6R_UNPACK_LOOP:
        ILDB    1,05
        ADDI    1,040
        MOVEM   1,0(04)
        ADDI    4,01
        SOJG    6,U_S6R_UNPACK_LOOP
U_S6R_OK:
        MOVEI   1,01
        MOVE    2,U_REC_LEN
        POPJ    17,
U_S6R_EOF:
        SETZ    1,
        SETZ    2,
        POPJ    17,
U_S6R_ERROR:
        SETO    1,
        SETZ    2,
        POPJ    17,

; AC1 = FD, AC2 = LENGTH, CHARS IN U_REC_CHARS.
U_S6R_WRITE:
        MOVEM   1,U_REC_FD
        MOVEM   2,U_REC_LEN
        CAILE   2,0170
        JRST    U_S6W_ERROR
        MOVEI   3,024
        MOVEI   4,U_REC_WORDS
U_S6W_CLEAR:
        SETZM   0(04)
        ADDI    4,01
        SOJG    3,U_S6W_CLEAR
        MOVE    6,02
        JUMPE   6,U_S6W_HEADER
        MOVE    5,U_S6BP0
        MOVEI   4,U_REC_WORDS
        HRR     5,04
        MOVEI   4,U_REC_CHARS
U_S6W_PACK_LOOP:
        MOVE    1,0(04)
        SUBI    1,040
        ANDI    1,077
        IDPB    1,05
        ADDI    4,01
        SOJG    6,U_S6W_PACK_LOOP
U_S6W_HEADER:
        MOVE    6,U_REC_LEN
        TLO     6,010000
        MOVEM   6,U_REC_HEADER
        MOVE    1,U_REC_FD
        MOVEI   2,U_REC_HEADER
        MOVEI   3,01
        PUSHJ   17,U_WRITE_WORDS
        CAIE    1,01
        JRST    U_S6W_ERROR
        MOVE    5,U_REC_LEN
        ADDI    5,05
        IDIVI   5,06
        JUMPE   5,U_S6W_OK
        MOVE    1,U_REC_FD
        MOVEI   2,U_REC_WORDS
        MOVE    3,05
        PUSHJ   17,U_WRITE_WORDS
        CAMN    1,05
        JRST    U_S6W_OK
U_S6W_ERROR:
        SETO    1,
        POPJ    17,
U_S6W_OK:
        SETZ    1,
        POPJ    17,

; READ ONE ASCII/UTF-9 LINE FROM FD IN AC1 INTO U_REC_CHARS.
; RETURNS AC1=1 LINE, 0 EOF, -1 ERROR; AC2=LENGTH.
U_LINE_READ:
        MOVEM   1,U_REC_FD
        SETZM   U_REC_LEN
U_LINE_LOOP:
        MOVE    1,U_REC_FD
        MOVE    2,U_BYTEBP
        MOVEI   3,01
        PUSHJ   17,U_READ_BYTES
        JUMPL   1,U_LINE_ERROR
        JUMPE   1,U_LINE_PHYS_EOF
        MOVE    4,U_BYTEWORD
        LSH     4,-033
        ANDI    4,0177
        CAIE    4,015
        JRST    U_LINE_NOT_CR
        JRST    U_LINE_LOOP
U_LINE_NOT_CR:
        CAIE    4,012
        JRST    U_LINE_CHAR
        MOVEI   1,01
        MOVE    2,U_REC_LEN
        POPJ    17,
U_LINE_CHAR:
        MOVE    5,U_REC_LEN
        CAIGE   5,0170
        MOVEM   4,U_REC_CHARS(05)
        CAIGE   5,0170
        ADDI    5,01
        MOVEM   5,U_REC_LEN
        JRST    U_LINE_LOOP
U_LINE_PHYS_EOF:
        SKIPN   U_REC_LEN
        JRST    U_LINE_EOF
        MOVEI   1,01
        MOVE    2,U_REC_LEN
        POPJ    17,
U_LINE_EOF:
        SETZ    1,
        SETZ    2,
        POPJ    17,
U_LINE_ERROR:
        SETO    1,
        SETZ    2,
        POPJ    17,

; WRITE U_REC_CHARS AS ASCII FOLLOWED BY CRLF. AC1=FD, AC2=LENGTH.
U_LINE_WRITE:
        MOVEM   1,U_REC_FD
        MOVE    6,02
        MOVEI   5,U_REC_CHARS
U_LINE_WRITE_LOOP:
        JUMPE   6,U_LINE_WRITE_CRLF
        MOVE    1,0(05)
        ANDI    1,0177
        LSH     1,033
        MOVEM   1,U_BYTEWORD
        MOVE    1,U_REC_FD
        MOVE    2,U_BYTEBP
        MOVEI   3,01
        PUSHJ   17,U_WRITE_BYTES
        CAIE    1,01
        JRST    U_LINE_ERROR
        ADDI    5,01
        SOJG    6,U_LINE_WRITE_LOOP
U_LINE_WRITE_CRLF:
        MOVE    6,U_CRLFWORD
        MOVEM   6,U_BYTEWORD
        MOVE    1,U_REC_FD
        MOVE    2,U_CRLFBP
        MOVEM   2,U_TMPBP
        MOVEI   2,U_TMPBP
        ; USE ORDINARY BYTE WRITES ONE CHARACTER AT A TIME FOR PORTABILITY.
        MOVEI   1,015
        PUSHJ   17,U_PUTC
        MOVEI   1,012
        PUSHJ   17,U_PUTC
        SETZ    1,
        POPJ    17,

.DATA
U_BYTEBP:
        .WORD   POINT 9,U_BYTEWORD,8
U_CRLFBP:
        .WORD   POINT 9,U_CRLFWORD,8
U_CRLFWORD:
        .WORD   015012000000
U_REC_HEADER:
        .WORD   0
U_REC_LEN:
        .WORD   0
U_REC_NWORDS:
        .WORD   0
U_REC_FD:
        .WORD   0
U_BYTEWORD:
        .WORD   0
U_TMPBP:
        .WORD   0

.BSS
U_REC_WORDS:
        .BLOCK  024
U_REC_CHARS:
        .BLOCK  0170
