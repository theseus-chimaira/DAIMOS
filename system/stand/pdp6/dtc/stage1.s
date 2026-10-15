/**
 * @file stage1.s
 * @brief PDP-6 DECtape Stage-1 loader for compressed or raw KINIT streams.
 *
 * Stage0 loads this loader from read-in paper tape at 000060. Stage-1 starts
 * DECtape unit 0 forward through Type-136 DCT0 and consumes one logical word
 * stream independent of physical DECtape block boundaries. Compressed input
 * is staged above the final image and decoded at 030000. The 32K profile uses
 * a raw image written directly at 030000 without any staging copy.
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
;       word 2      compressed_words,,mode
;       remainder   grouped D6 codec payload or raw image words
;
; Mode 0 is the original contiguous stream. Mode 1 (compressed_words=0)
; embeds the stream within an ordinary TSFS V1.1 member: word 3 gives the
; first physical tail block, words 4..127 carry 124 raw KINIT words, and
; the remaining words occupy reserved blocks beginning at that tail block.
; Blocks 1..tail-1 are normal TSFS descriptors and metadata and are skipped
; sequentially. Stage1 requires no TSFS parser or additional image buffer.
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
        hrrz 06,03
        ; Zero compressed length selects the direct 32K image stream.
        ; Like the DSK lowmem loader, this needs no second staging copy.
        movei 01,030000
        move 02,013
        jumpn 04,compressed_stream
        jumpe 06,load_loop
        caie 06,1
        jrst bad_tape
        jrst hybrid_stream
compressed_stream:
        jumpn 06,bad_tape
        ; Normal compressed images stage the payload above their output.
        add 01,013
        move 02,04
load_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,load_loop
        jumpe 04,stage1_raw_handoff
        jrst stage1_handoff

; A TSFS bootable member puts its normal redundant descriptors in blocks 1
; and 2 and its table directory at block 3.  Skip these (and any file/extent
; metadata blocks) without copying them into RAM.  The host formatter has
; reserved the tail extent before allocating ordinary file data.
hybrid_stream:
        pushj 017,read_word
        move 014,03               ; physical tail block
        caige 014,4
        jrst bad_tape
        caile 014,01101           ; TSFS has 01102 blocks per member
        jrst bad_tape
        movei 010,0174           ; 128 - 4 header words
        camle 010,02
        move 010,02
hybrid_prefix_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        subi 02,1
        sojg 010,hybrid_prefix_loop
        jumpe 02,stage1_raw_handoff

        ; We have just finished physical block zero.  The next tail word is
        ; at block 014, so discard exactly (014-1)*0200 metadata words.
        move 07,014
        subi 07,1
        lsh 07,7
hybrid_skip_loop:
        pushj 017,read_word
        sojg 07,hybrid_skip_loop
hybrid_tail_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,hybrid_tail_loop
        jrst stage1_raw_handoff

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

        ; Save the entry PC and output end: grouped decoding uses AC5 and
        ; AC10..16 for its history/copy engine. The Stage-1 stack remains live.
        push 017,05
        movei 03,030000
        add 03,013                    ; compressed stream begins at end of image
        move 01,03                    ; staged source above KINIT output
        move 02,04                    ; complete compressed-word count
        push 017,03                   ; save uncompressed image end
        movei 03,030000
        move 04,(017)                 ; saved exclusive output end
        movei 05,030000
        pushj 017,d6lz_fixed_base
        jumpn 01,bad_tape
        pop 017,012                   ; restore KINIT stack base
        pop 017,05                    ; restore entry point
        move 017,012
        setz 01,
        setz 02,
        jrst 0(05)

; The raw stream is already resident at 030000.  Enter without installing
; the decompressor or maintaining a compressed image beyond 32K.
stage1_raw_handoff:
        movei 017,030000
        add 017,013
        setz 01,
        setz 02,
        jrst 0(05)

; This diagnostic must remain above the decoder destination.  Decoder failure
; is detected only after 000060..000127 has been overwritten by the installed
; low-core image.
bad_tape:
        jrst stage1_fail_b1
        .include "../common/stage1-error.inc"
