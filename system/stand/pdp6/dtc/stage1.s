/**
 * @file stage1.s
 * @brief PDP-6 DECtape Stage-1 loader for a sequential compressed KINIT stream.
 *
 * Stage0 loads this loader from read-in paper tape at 000060. Stage-1 starts
 * DECtape unit 0 forward through Type-136 DCT0 and consumes one logical word
 * stream independent of physical DECtape block boundaries. Compressed input is
 * staged immediately above the declared final image, the common decoder is
 * installed at 000060, and KINIT is expanded at 030000.
 *
 * Header validation requires nonzero image/compressed lengths and an entry
 * offset strictly inside the final image. At handoff AC17 points above the
 * expanded KINIT and AC1/AC2 are cleared; other scratch ACs are non-contractual.
 */
;
; Stage1 starts DECtape unit 0
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
        ; Keep the pushdown list below the KINIT output window.  The full
        ; compressed image expands from 030000 upward and would overwrite a
        ; historical 070000 stack before the decoder returns.
        movei 017,020000

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
        jrst stage1_handoff

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
        jrst bad_tape

daimon_magic: .word 0444151555756
        .include "../common/decompressor.inc"

; Keep the handoff above the installed decoder destination (000060..000127).
; Production Stage0 loads this Stage1 at 000060, so executing the handoff from
; the early part of the loader would be overwritten by the BLT itself.
stage1_handoff:
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
        ; Match DSK/DRM handoff semantics: KINIT's bootstrap pushdown list
        ; begins immediately after the expanded image.  Leaving AC17 on the
        ; low Stage1 stack would let kcore_load overwrite live return words.
        move 017,012
        setz 01,
        setz 02,
        jrst 0(05)

; This diagnostic must remain above the decoder destination.  Decoder failure
; is detected only after 000060..000127 has been overwritten by the installed
; low-core image.
bad_tape:
        jrst stage1_fail_b1
        .include "../common/stage1-error.inc"
