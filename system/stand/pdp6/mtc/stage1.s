; stage1.s -- minimal opaque-image Stage1 for PDP-6 magnetic tape.
;
; Stage0 loads this loader from RIM paper tape. Stage1 reads one magnetic-tape record
; through a Type 516 control and Type 136 data control directly to 040000.
; The record boundary terminates the opaque image; execution starts at 040000.
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
        movei 017,070000

        ; Type 136: input, six 6-bit characters, device 3, move enabled.
        cono 0200,004000

        ; Type 516: unit 0, 556 bpi, binary parity, read forward.
        cono 0220,052400

        movei 01,040000
read_loop:
        conso 0200,001000
        jrst read_wait
        datai 0200,0(01)
        aoj 01,
        jrst read_loop

read_wait:
        ; PARITY_ERR is intentionally omitted; see file comment above.
        consz 0224,0000120
        jrst read_error
        consz 0224,0400400
        jrst bad_tape
        conso 0224,0000004
        jrst read_loop
        ; EOR may precede delivery of the final DCT word.
        consz 0200,0002000
        jrst read_loop

        ; At least one opaque image word must have been transferred.
        caie 01,040000
        jrst mtc_install_decoder
bad_tape:
        jrst stage1_fail_b1

mtc_install_decoder:
        movei 01,d6lz_image_start
        hrl 01,01
        hrri 01,d6lz_fixed_base
        blt 01,d6lz_fixed_base+(d6lz_image_end-d6lz_image_start)-1
        jrst 040000

read_error:
        jrst stage1_fail_b1

        .include "../common/stage1-error.inc"
        .include "../common/decompressor.inc"
