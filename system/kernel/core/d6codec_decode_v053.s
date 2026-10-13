/* D6 codec v053 decoder. Staged only; not yet linked into KCORE. */
        .text
        .globl d6codec_v053_decode
d6codec_v053_decode:
        move    5,3                     ; output origin
        add     4,3                     ; output end pointer
        jumpe   2,d6codec_v053_empty
d6codec_v053_group:
        move    6,0(1)                  ; control
        aoj     1,
        soj     2,
        move    7,6
        lsh     7,-036                  ; token count
        jumpe   7,d6codec_v053_bad
        caile   7,036
        jrst    d6codec_v053_bad
        move    10,6
        lsh     10,6                    ; flags at high bits
        move    11,10
        move    12,7
        setz    13,                    ; match count
 d6codec_v053_popcount:
        skipge  11
        aoj     13,
        lsh     11,1
        sojg    12,d6codec_v053_popcount
        move    11,13
        aoj     11,
        lsh     11,-1                  ; descriptor words
        move    12,7
        sub     12,13                  ; literal words
        add     12,11                  ; entire group payload
        camle   12,2
        jrst    d6codec_v053_bad
        sub     2,12
        move    14,1                   ; descriptor cursor
        move    15,1
        add     15,11                  ; literal cursor
        add     1,12                   ; next group
        setz    11,                    ; descriptor half toggle
 d6codec_v053_token:
        jumpl   10,d6codec_v053_match
        camge   3,4
        jrst    d6codec_v053_lit_ok
        jrst    d6codec_v053_bad
 d6codec_v053_lit_ok:
        move    0,0(15)
        movem   0,0(3)
        aoj     15,
        aoj     3,
        jrst    d6codec_v053_next
 d6codec_v053_match:
        move    0,0(14)
        jumpe   11,d6codec_v053_left
        hrrz    0,0
        aoj     14,
        setz    11,
        jrst    d6codec_v053_desc
 d6codec_v053_left:
        hlrz    0,0
        movei   11,1
 d6codec_v053_desc:
        move    12,0
        lsh     12,-11                 ; length = bits 9..15
        andi    12,0177
        jumpe   12,d6codec_v053_bad
        move    16,3
        add     16,12
        camle   16,4
        jrst    d6codec_v053_bad
        move    13,0
        andi    13,0777                ; distance
        move    16,0
        lsh     16,-020                ; 2-bit kind
        jumpe   16,d6codec_v053_copy
        caie    16,1
        jrst    d6codec_v053_half
        jumpn   13,d6codec_v053_bad
 d6codec_v053_zero:
        setzm   0(3)
        aoj     3,
        sojg    12,d6codec_v053_zero
        jrst    d6codec_v053_next
; Kind 2: 18-bit halfword match distance. For even distance,
; the two halves align with entire output words, so ordinary overlapping
; word copy is correct. Odd distance needs separate halfword handling.
 d6codec_v053_half:
        caie    16,2
        jrst    d6codec_v053_bad
        jumpe   13,d6codec_v053_bad
        trne    13,1
        jrst    d6codec_v053_bad
        lsh     13,-1            ; even halfword distance -> words
        jrst    d6codec_v053_copy
; Odd halfword offset: source begins in the low half of a word.
; First write the high half to the destination before reading the low
; half from source+1. This makes distance-one overlapping copies valid.
 d6codec_v053_copy:
        jumpe   13,d6codec_v053_bad
        move    16,3
        sub     16,13
        camge   16,5
        jrst    d6codec_v053_bad
 d6codec_v053_copy_loop:
        move    0,0(16)
        movem   0,0(3)
        aoj     16,
        aoj     3,
        sojg    12,d6codec_v053_copy_loop
 d6codec_v053_next:
        lsh     10,1
        sojg    7,d6codec_v053_token
        camn    3,4
        jrst    d6codec_v053_finish
        jumpn   2,d6codec_v053_group
 d6codec_v053_bad:
        seto    1,
        popj    17,
 d6codec_v053_empty:
        came    3,4
        jrst    d6codec_v053_bad
        jrst    d6codec_v053_finish
 d6codec_v053_finish:
        jumpn   2,d6codec_v053_bad
        setz    1,
        popj    17,
