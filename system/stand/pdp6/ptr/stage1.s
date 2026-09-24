; stage1.s -- two-tape opaque-image Stage1 for PDP-6 paper tape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 then reads two raw
; five-byte 36-bit-word streams.  Each tape starts with a word count.
;
; Tape 1:
;       word 0      payload word count
;       words 1..   first opaque payload -> 040000...
;
; After Tape 1 Stage1 prints "TAPE2 " through the installed SIXBIT helper,
; resets the reader, and waits for Tape 2.  Tape 2 is
; loaded immediately after the Tape 1 high-memory payload:
;
; Tape 2:
;       word 0      payload word count
;       remainder   second opaque payload  -> 040000 + tape1_count
;
; When Tape 2 is complete Stage1 enters the first payload at 040000.  Image
; contents are deliberately opaque to Stage1; there is no image validation.
;
; The PDP-6 Type 760 PTR interface has no reader-error status bit, so a reader
; that cannot produce the next character simply never raises DONE.

        .text
        .globl start
        .globl __start

__start:
start:
        setom 000040
        setom 000041
        movei 017,070000
        movei 01,bootstrap_sixbit_image
        hrl 01,01
        hrri 01,077760
        blt 01,077776
        cono 0104,0020

; Read Tape 1 count.
        pushj 017,read_word

; Load Tape 1 contiguously at 040000.
        move 02,03
        movei 01,040000
        pushj 017,read_words
        move 07,01

        ; One word is enough to tell the operator why the reader stopped.
        move 01,msg_tape2
        pushj 017,077760

; Reset/start the reader for Tape 2.  If no tape is present, the normal
; PTR wait loop simply waits until the operator supplies one.
        cono 0104,0020
        pushj 017,read_word
        move 02,03
        move 01,07
        pushj 017,read_words
        movei 01,d6lz_image_start
        hrl 01,01
        hrri 01,d6lz_fixed_base
        blt 01,d6lz_fixed_base+(d6lz_image_end-d6lz_image_start)-1
        jrst 040000

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
        coni 0104,tmp
        move 06,tmp
        trnn 06,0010
        jrst read_byte_loop
        datai 0104,ioword
        move 05,ioword
        andi 05,0377
        lsh 03,010
        ior 03,05
        sojg 04,read_byte_loop
        popj 017,

ioword:      .word 0
tmp:         .word 0
msg_tape2:   .word 0644160452200        ; "TAPE2 "
bootstrap_sixbit_image:
        .include "../common/sixbit-fixed.inc"

        .include "../common/decompressor.inc"
