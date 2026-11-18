; v1659 opt-in, test-linked bitmap codec. Instruction stream is the
; validated 30-word v1451 testkit encoder/decoder, copied without
; modification except for this comment and versioned filename.
;
; Encoder: AC1 raw base, AC2 positive RAW words, AC3 packed base.
; Decoder: AC1 packed base, AC3 output base, AC4 positive RAW words.
; Trusted disjoint MM-owned physical extents; NO bounds checking here.
; The v1659 C probe computes exact output capacity and verifies all
; encoded words before any MM release. AC10..AC15 preserved.
        .text
        .globl bitmap_encode_v1451
        .globl bitmap_decode_v1451

bitmap_encode_v1451:
bitmap_enc_group:
        move    5,3
        setzm   (3)
        aoj     3,
        movsi   6,0400000
bitmap_enc_word:
        move    0,(1)
        aoj     1,
        jumpe   0,bitmap_enc_advance
        movem   0,(3)
        aoj     3,
        iorm    6,(5)
bitmap_enc_advance:
        soje    2,bitmap_enc_finish
        lsh     6,-1
        jumpn   6,bitmap_enc_word
bitmap_enc_finish:
        jumpn   2,bitmap_enc_group
bitmap_enc_done:
        popj    17,

bitmap_decode_v1451:
bitmap_dec_group:
        move    6,(1)
        aoj     1,
        movsi   7,0400000
bitmap_dec_word:
        setzm   (3)
        tdnn    6,7
        jrst    bitmap_dec_advance
        move    0,(1)
        aoj     1,
        movem   0,(3)
bitmap_dec_advance:
        aoj     3,
        soje    4,bitmap_dec_done
        lsh     7,-1
        jumpn   7,bitmap_dec_word
        jrst    bitmap_dec_group
bitmap_dec_done:
        popj    17,
