; stage1.s -- sequential opaque-image Stage1 for PDP-6 magnetic tape.
;
; Stage0 loads this loader from RIM paper tape. Stage1 reads one magnetic-tape
; record through a Type 516 control and Type 136 data control. The record is a
; six-character-per-word stream containing:
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

        ; Type 136: input, six 6-bit characters, device 3, move enabled.
        movei 01,004000
        cono 0200,0(01)

        ; Type 516: unit 0, 556 bpi, binary parity, read forward.
        movei 01,052400
        cono 0220,0(01)

        movei 01,header
        movei 02,02
        pushj 017,read_words
        move 02,header
        camn 02,daimon_magic
        jrst header_ok
        jrst fail
header_ok:
        hlrz 02,header+01
        jumpe 02,fail
        movem 02,image_words
        hrrz 03,header+01
        caml 03,02
        jrst fail
        movem 03,entry_off
        movei 04,040000
        add 04,02
        caile 04,060000
        jrst fail
        movei 01,040000
        pushj 017,read_words
        movei 02,040000
        add 02,entry_off
        movem 02,entry_addr
        movei 017,050000
        setz 01,
        setz 02,
        jrst @entry_addr

read_words:
        jumpe 02,read_done
read_loop:
        conso 0200,001000
        jrst read_loop
        datai 0200,ioword
        move 03,ioword
        movem 03,0(01)
        aoj 01,
        sojg 02,read_loop
read_done:
        popj 017,

fail:
        halt .
        jrst fail

header:      .block 02
image_words: .word 0
entry_off:   .word 0
entry_addr:  .word 0
ioword:      .word 0

daimon_magic: .word 0444151555756
