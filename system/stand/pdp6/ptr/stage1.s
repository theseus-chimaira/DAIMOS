; stage1.s -- sequential opaque-image Stage1 for PDP-6 paper tape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 then reads a five-byte
; 36-bit word stream containing:
;       word 0  DAIMON magic
;       word 1  image_words,,entry_offset
;       word 2  opaque image word 0
; The image is loaded contiguously at 040000.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000
        movei 01,0020
        cono 0104,0(01)
        .include "../common/sequential-load.inc"

read_words:
read_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,read_loop
        popj 017,

read_word:
        setz 03,
        movei 04,05
read_byte_loop:
        pushj 017,ptr_getc
        lsh 03,010
        ior 03,05
        sojg 04,read_byte_loop
        popj 017,

ptr_getc:
ptr_wait:
        coni 0104,tmp
        move 06,tmp
        trnn 06,0010
        jrst ptr_wait
        datai 0104,ioword
        move 05,ioword
        andi 05,0377
        popj 017,

fail:
        halt .
        jrst fail

header:      .block 02
ioword:     .word 0
tmp:        .word 0
daimon_magic: .word 0444151555756
