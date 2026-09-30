/**
 * @file stage1.s
 * @brief PDP-6 Type-516 magnetic-tape Stage-1 loader for compressed KINIT.
 *
 * Stage0 loads this image at 000060. Stage-1 reads unit 0 through Type-516 and
 * Type-136, continuing across tape-record boundaries until the declared
 * compressed word count is satisfied. The payload is staged above final KINIT
 * and expanded at 030000 by the common decoder copied to low core.
 *
 * Header validation requires nonzero image/compressed lengths and entry_offset
 * strictly inside the image. The historical SIMH parity-error indication is
 * intentionally ignored because it is spuriously asserted on valid legacy
 * seven-track images. AC17 is reset above KINIT immediately before entry.
 */
;
; Stage1 reads one magnetic-tape
; record through a Type 516 control and Type 136 data control.  The record is:
;
;       word 0      SIXBIT DAIMON
;       word 1      uncompressed_words,,entry_offset
;       word 2      compressed_words,,0
;       remainder   D6LZ36 payload
;
; Compressed input is staged immediately after the final image range, expanded
; at 030000 by the shared fixed low-core decoder, and entered at the relative
; entry point.
;
; Do not test the Type 516 PARITY_ERR bit here. The historical PDP-6 SIMH
; 7-track implementation asserts it spuriously on ordinary legacy tape images.

        .text
        .globl start
        .globl __start

__start:
start:
        setom 000040
        setom 000041
        ; Keep the pushdown list below the KINIT output window.  A 070000
        ; return word lies inside the current full KINIT image and is destroyed
        ; during decompression.
        movei 017,020000

        ; Type 136: input, six 6-bit characters, device 3, move enabled.
        cono 0200,004000

        ; Type 516: unit 0, 556 bpi, binary parity, read forward.
        cono 0220,052400

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

        movei 01,030000
        add 01,013
        move 02,04
load_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,load_loop
        jrst stage1_handoff

; Return the next 36-bit DCT word in AC3.  Reaching EOR before the declared
; payload is complete is a truncated-image failure.
read_word:
        conso 0200,001000
        jrst read_wait
        datai 0200,03
        popj 017,

read_wait:
        ; PARITY_ERR is intentionally omitted; see file comment above.
        consz 0224,0000120
        jrst read_error
        consz 0224,0400400
        jrst bad_tape
        ; Large boot files span multiple SIMH tape records because the PDP-6
        ; MTC device buffer is 32 KiB.  EOR means the current record is fully
        ; consumed; restart READ on the next record and continue satisfying the
        ; exact word count from the compressed-image header.
        consz 0224,0000004
        cono 0220,052400
        jrst read_word

read_error:
        jrst bad_tape
daimon_magic: .word 0444151555756
        .include "../common/decompressor.inc"

; Execute the decoder installation from above its low-core destination.
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
        move 017,012                  ; KINIT stack starts at image end
        jrst 0(05)

bad_tape:
        .include "../common/stage1-error.inc"
