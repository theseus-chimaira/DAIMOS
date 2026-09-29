; stage1.s -- two-tape D6LZ36 Stage1 for PDP-6 paper tape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 then reads two raw
; five-byte 36-bit-word streams.  Each tape starts with a word count.
;
; Tape 1:
;       word 0      tape payload word count
;       word 1      SIXBIT DAIMON
;       word 2      uncompressed_words,,entry_offset
;       word 3      compressed_words,,0
;       words 4..   first compressed segment
;
; After Tape 1 Stage1 prints "TAPE2 " through the installed SIXBIT helper,
; resets the reader, and waits for Tape 2.  Tape 2 is
; loaded immediately after the Tape 1 compressed segment:
;
;       word 0      remaining compressed word count
;       remainder   second compressed segment
;
; The two compressed segments are staged contiguously immediately above the
; final uncompressed image, expanded at 030000 by the shared fixed low-core
; decoder, then entered at the header-relative entry point.
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
        ; Keep the pushdown list below the KINIT output window.  The full image
        ; expands through 070000, so a stack there corrupts the decoder return.
        movei 017,020000
        movei 01,bootstrap_sixbit_image
        hrl 01,01
        hrri 01,077760
        blt 01,077776
        cono 0104,0020

        ; Read Tape 1 count and the normal compressed-image header.
        pushj 017,read_word
        move 02,03
        pushj 017,read_word
        came 03,daimon_magic
        jrst bad_tape
        pushj 017,read_word
        hlrz 07,03
        movem 07,image_words
        hrrz 016,03
        addi 016,030000
        pushj 017,read_word
        hlrz 07,03
        movem 07,compressed_words
        subi 02,03
        movei 01,030000
        add 01,image_words
        pushj 017,read_words
        movem 01,stage_ptr

        ; One word is enough to tell the operator why the reader stopped.
        move 01,msg_tape2
        pushj 017,077760

; Reset/start the reader for Tape 2.  If no tape is present, the normal
; PTR wait loop simply waits until the operator supplies one.
        cono 0104,0020
        pushj 017,read_word
        move 02,03
        move 01,stage_ptr
        pushj 017,read_words
        jrst stage1_handoff

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
        datai 0104,05
        andi 05,0377
        lsh 03,010
        ior 03,05
        sojg 04,read_byte_loop
        popj 017,

tmp:         .word 0
msg_tape2:   .word 0644160452200        ; "TAPE2 "
daimon_magic:.word 0444151555756
bootstrap_sixbit_image:
        .include "../common/sixbit-fixed.inc"

        .include "../common/decompressor.inc"

; Production Stage0 loads PTR Stage1 at 000060.  Keep the decoder-copy and
; post-copy instructions above the installed 000060 decoder image.
stage1_handoff:
        movei 01,d6lz_image_start
        hrl 01,01
        hrri 01,d6lz_fixed_base
        blt 01,d6lz_fixed_base+(d6lz_image_end-d6lz_image_start)-1

        movei 012,030000
        move 013,image_words
        move 03,012
        add 03,013
        move 04,compressed_words
        move 014,012
        setz 011,
        pushj 017,d6lz_fixed_base
        jumpn 00,bad_tape
        jumpn 04,bad_tape
        move 017,012                  ; KINIT stack starts at image end
        jrst 0(016)

; Keep the post-decode failure target above the installed decoder image.
bad_tape:
        halt .
        .bss
image_words:      .block 1
compressed_words: .block 1
stage_ptr:        .block 1
