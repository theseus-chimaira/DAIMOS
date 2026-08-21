; stage1.s -- two-tape opaque-image Stage1 for PDP-6 paper tape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 then reads two raw
; five-byte 36-bit-word streams.  Each tape starts with a word count.
;
; Tape 1:
;       word 0      payload word count
;       words 1..   first opaque payload -> 040000...
;
; After Tape 1 Stage1 prints "CHANGE TAPE" through the SIXBIT routine just
; already installed by Stage1, resets the reader, and waits for Tape 2.  Tape 2 is
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
        movei 017,050000
        pushj 017,install_bootstrap_sixbit
        movei 01,0020
        cono 0104,0(01)

; Read Tape 1 count.
        pushj 017,read_word
        movem 03,tape1_count

; Load Tape 1 contiguously at 040000.
        move 02,tape1_count
        movei 01,040000
        pushj 017,read_words
        movem 01,tape2_base

        move 01,msg_change0
        pushj 017,077760
        move 01,msg_change1
        pushj 017,077760

; Terminate the operator message with CR/LF.  The SIXBIT helper waits for
; the final character, so CR may be written immediately; wait only between
; CR and LF.
        movei 03,015
        datao 0120,03
change_cr_wait:
        coni 0120,04
        trne 04,0020
        jrst change_cr_wait
        movei 03,012
        datao 0120,03

; Reset/start the reader for Tape 2.  If no tape is present, the normal
; PTR wait loop simply waits until the operator supplies one.
        movei 01,0020
        cono 0104,0(01)
        pushj 017,read_word
        move 02,03
        move 01,tape2_base
        pushj 017,read_words
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

tape1_count: .word 0
tape2_base:  .word 0
ioword:      .word 0
tmp:         .word 0
msg_change0: .word 0435041564745
msg_change1: .word 0006441604500
        .include "../common/bootstrap-sixbit-077760.inc"
