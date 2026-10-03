; DXRINFO -- DISPLAY A DAIMOS DXR V1 HEADER.
.ENTRY START
.TEXT
START:
        MOVEM   1,U_ARGC
        MOVEM   2,U_ARGV
        CAIE    1,02
        JRST    DXR_FAIL
        MOVEI   1,01
        PUSHJ   17,U_ARG
        SETZ    2,
        UUO     041,0(1)
        JUMPL   1,DXR_FAIL
        MOVE    10,01
        MOVE    1,010
        MOVEI   2,DXR_HEAD
        MOVEI   3,02
        PUSHJ   17,U_READ_WORDS
        CAIE    1,02
        JRST    DXR_FAIL_CLOSE
        MOVE    4,DXR_HEAD
        HLRZ    5,04
        CAIE    5,0447062
        JRST    DXR_FAIL_CLOSE
        MOVEI   1,DXR_ENTRY_MSG
        PUSHJ   17,U_PUTS6
        HRRZ    1,DXR_HEAD
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        MOVEI   1,DXR_IMAGE_MSG
        PUSHJ   17,U_PUTS6
        HLRZ    1,DXR_HEAD+01
        MOVEM   1,DXR_IMAGE
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        MOVEI   1,DXR_BSS_MSG
        PUSHJ   17,U_PUTS6
        HRRZ    1,DXR_HEAD+01
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        MOVEI   1,DXR_RELOC_MSG
        PUSHJ   17,U_PUTS6
        MOVE    2,DXR_IMAGE
        ADDI    2,043
        SETZ    1,
        IDIVI   1,044
        PUSHJ   17,U_PUTOCT6
        PUSHJ   17,U_CRLF
        PUSHJ   17,DXR_CLOSE
        SETZ    1,
        PUSHJ   17,U_EXIT
DXR_FAIL_CLOSE:
        PUSHJ   17,DXR_CLOSE
DXR_FAIL:
        MOVEI   1,01
        PUSHJ   17,U_EXIT
DXR_CLOSE:
        MOVE    1,010
        UUO     042,0(1)
        POPJ    17,
.DATA
DXR_ENTRY_MSG: .WORD 06
        .SIXBIT /ENTRY /
DXR_IMAGE_MSG: .WORD 06
        .SIXBIT /IMAGE /
DXR_BSS_MSG: .WORD 06
        .SIXBIT /BSS   /
DXR_RELOC_MSG: .WORD 06
        .SIXBIT /RELOC /
DXR_HEAD: .WORD 0
        .WORD 0
DXR_IMAGE: .WORD 0
.INCLUDE "BASE.S"
