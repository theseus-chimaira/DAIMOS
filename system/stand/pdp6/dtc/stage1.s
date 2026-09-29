; stage1.s -- D6LZ36 boot-image Stage1 for PDP-6 DECtape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 starts DECtape unit 0
; reading forward through the Type 136 data control.  The DECtape stream is:
;
;       word 0      SIXBIT DAIMON
;       word 1      uncompressed_words,,entry_offset
;       word 2      compressed_words,,0
;       remainder   D6LZ36 payload
;
; The compressed stream is staged immediately after its final image range,
; expanded at 030000 by the shared fixed low-core decoder, then entered at the
; header-relative entry point.  Physical DECtape block boundaries are handled
; by the controller; Stage1 never buffers or restarts per block.
;
; Every fatal loader/media failure prints the compact halfword diagnostic ?B1
; and halts.  Stage1 deliberately does not spend words on detailed errors.

        .text
        .globl start
        .globl __start

__start:
start:
        setom 000040
        setom 000041
        movei 017,070000

        ; DCT0: device 1 (DTC), device -> processor, move enabled.
        cono 0200,004040

        ; DTC0: selected, start forward, READ DATA.
        cono 0210,0220300

        ; Validate the normal compressed-image header.
        pushj 017,read_word
        came 03,daimon_magic
        jrst bad_tape
        pushj 017,read_word
        hlrz 013,03
        jumpe 013,bad_tape
        hrrz 05,03
        caml 05,013
        jrst bad_tape
        addi 05,030000
        pushj 017,read_word
        hlrz 04,03
        jumpe 04,bad_tape

        ; Stage compressed input above the final uncompressed image.
        movei 01,030000
        add 01,013
        move 02,04
load_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,load_loop

        movei 01,d6lz_image_start
        hrl 01,01
        hrri 01,d6lz_fixed_base
        blt 01,d6lz_fixed_base+(d6lz_image_end-d6lz_image_start)-1

        movei 012,030000
        move 03,012
        add 03,013
        move 014,012
        setz 011,
        pushj 017,d6lz_fixed_base
        jumpn 00,bad_tape
        jumpn 04,bad_tape
        setz 01,
        setz 02,
        jrst 0(05)


; Return the next 36-bit DCT word in AC3.  DTC status B distinguishes genuine
; controller/data errors from reaching the end zone before the declared image
; length has been satisfied.
read_word:
        conso 0200,001000
        jrst read_wait
        datai 0200,03
        popj 017,

read_wait:
        consz 0214,0000034
        jrst read_error
        consz 0214,0000002
        jrst bad_tape
        jrst read_word

read_error:
bad_tape:
        jrst stage1_fail_b1

daimon_magic: .word 0444151555756
        .include "../common/stage1-error.inc"
        .include "../common/decompressor.inc"
