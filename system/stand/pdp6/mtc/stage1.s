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

        .include "../common/sequential-load.inc"

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
entry_addr:  .word 0
ioword:      .word 0

daimon_magic: .word 0444151555756
