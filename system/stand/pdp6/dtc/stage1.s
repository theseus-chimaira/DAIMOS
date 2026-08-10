; stage1.s -- block-oriented opaque-image Stage1 for PDP-6 DECtape.
;
; Stage0 loads this loader from RIM paper tape.  The opaque boot stream begins
; in DECtape block 0 and consists of:
;       word 0  SIXBIT /DAIMON/
;       word 1  image_words,,entry_offset
;       word 2  opaque image word 0
; The image is loaded at 040000.  Only enough consecutive 128-word DECtape
; blocks to contain the header and image are read.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000

        ; Read block 0 into the temporary block buffer.
        pushj 017,read_block

        move 02,blockbuf
        camn 02,daimon_magic
        jrst magic_ok
        jrst fail

magic_ok:
        move 02,blockbuf+1
        hlrz 03,02
        jumpe 03,fail
        movem 03,image_words
        hrrz 04,02
        caml 04,03
        jrst fail
        movem 04,entry_off

        movei 05,040000
        add 05,03
        caile 05,060000
        jrst fail

        ; Copy the payload portion of block 0.
        movei 01,040000
        move 02,image_words
        movei 06,blockbuf+2
        movei 07,0176
        pushj 017,copy_words

next_block:
        jumpe 02,image_done
        pushj 017,read_block
        movei 06,blockbuf
        movei 07,0200
        pushj 017,copy_words
        jrst next_block

image_done:
        cono 0210,0
        cono 0200,0
        movei 02,040000
        add 02,entry_off
        movem 02,entry_addr
        movei 017,050000
        setz 01,
        setz 02,
        jrst @entry_addr

; Copy min(AC2, AC7) words from (AC6) to (AC1).
; AC1 and AC6 advance; AC2 is the remaining image word count.
copy_words:
        jumpe 02,copy_done
        jumpe 07,copy_done
        move 03,0(06)
        movem 03,0(01)
        aoj 01,
        aoj 06,
        soj 02,
        sojg 07,copy_words
copy_done:
        popj 017,

; Read the next physical DECtape data block into blockbuf.
read_block:
        ; DCT0: device 1 (DTC), device -> processor, move enabled.
        movei 10,004040
        cono 0200,0(10)

        ; DTC0: selected, start forward, READ DATA.
        movei 10,0220300
        cono 0210,0(10)

        movei 11,blockbuf
        movei 12,0200
read_word:
read_wait:
        conso 0200,001000
        jrst read_wait
        datai 0200,0(11)
        aoj 11,
        sojg 12,read_word

        ; Wait for block completion and reject controller errors.
read_done_wait:
        coni 0214,ioword
        move 10,ioword
        trne 10,0000030
        jrst fail
        trnn 10,0000001
        jrst read_done_wait
        cono 0210,0
        cono 0200,0
        popj 017,

fail:
        cono 0210,0
        cono 0200,0
        halt .
        jrst fail

image_words: .word 0
entry_off:   .word 0
entry_addr:  .word 0
ioword:      .word 0
daimon_magic:.word 0444151555756
blockbuf:    .space 0200
