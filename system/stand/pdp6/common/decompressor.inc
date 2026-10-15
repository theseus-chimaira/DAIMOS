/**
 * @file decompressor.inc
 * @brief Sole grouped-control 36-bit decoder installed at executive 000060.
 *
 * Stage-1 includes these instructions at a temporary source address and
 * copies them to 000060. Every intra-decoder branch uses an absolute target
 * assembled relative to that *installed* image, not its source location.
 * KCORE assembles this identical source at 000060 for runtime use.
 *
 * ABI: AC1 source cursor; AC2 remaining compressed words; AC3 decoded cursor;
 * AC4 exclusive output end; AC5 fixed output origin; AC17 stack pointer.
 * AC1=0 on exact successful completion, +1 at whole-group NEED_INPUT,
 * -1 on malformed input. AC3 advances. AC10..16 are caller-saved here.
 * Input must contain whole groups; Stage-1 uses a complete image and KCORE's
 * VFS frontend passes exactly one buffered group per invocation.
 */
        .equ d6lz_fixed_base,000060
        .globl d6codec_fixed_decode_core
        .globl d6lz_image_start
        .globl d6lz_image_end
d6codec_fixed_decode_core:
d6lz_image_start:
        caml    3,5
        jrst    d6lz_fixed_base+(d6codec_v061_start_ok-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
d6codec_v061_start_ok:
        camle   3,4
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        jumpe   2,d6lz_fixed_base+(d6codec_v061_no_input-d6lz_image_start)
d6codec_v053_group:
        move    6,0(1)                  ; control
        aoj     1,
        soj     2,
        move    7,6
        lsh     7,-036                  ; token count
        jumpe   7,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        caile   7,036
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        move    10,6
        lsh     10,6                    ; flags at high bits
        move    11,10
        move    12,7
        setz    13,                    ; match count
 d6codec_v053_popcount:
        skipge  11
        aoj     13,
        lsh     11,1
        sojg    12,d6lz_fixed_base+(d6codec_v053_popcount-d6lz_image_start)
        move    11,13
        aoj     11,
        lsh     11,-1                  ; descriptor words
        move    12,7
        sub     12,13                  ; literal words
        add     12,11                  ; entire group payload
        camle   12,2
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        sub     2,12
        move    14,1                   ; descriptor cursor
        move    15,1
        add     15,11                  ; literal cursor
        add     1,12                   ; next group
        setz    11,                    ; descriptor half toggle
 d6codec_v053_token:
        jumpl   10,d6lz_fixed_base+(d6codec_v053_match-d6lz_image_start)
        camge   3,4
        jrst    d6lz_fixed_base+(d6codec_v053_lit_ok-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
 d6codec_v053_lit_ok:
        move    0,0(15)
        movem   0,0(3)
        aoj     15,
        aoj     3,
        jrst    d6lz_fixed_base+(d6codec_v053_next-d6lz_image_start)
 d6codec_v053_match:
        move    0,0(14)
        jumpe   11,d6lz_fixed_base+(d6codec_v053_left-d6lz_image_start)
        hrrz    0,0
        aoj     14,
        setz    11,
        jrst    d6lz_fixed_base+(d6codec_v053_desc-d6lz_image_start)
 d6codec_v053_left:
        hlrz    0,0
        movei   11,1
 d6codec_v053_desc:
        move    12,0
        lsh     12,-11                 ; length = bits 9..15
        andi    12,0177
        jumpe   12,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        move    16,3
        add     16,12
        camle   16,4
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        move    13,0
        andi    13,0777                ; distance
        move    16,0
        lsh     16,-020                ; 2-bit kind
        jumpe   16,d6lz_fixed_base+(d6codec_v053_copy-d6lz_image_start)
        caie    16,1
        jrst    d6lz_fixed_base+(d6codec_v053_half-d6lz_image_start)
        jumpn   13,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
 d6codec_v053_zero:
        setzm   0(3)
        aoj     3,
        sojg    12,d6lz_fixed_base+(d6codec_v053_zero-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6codec_v053_next-d6lz_image_start)
; Kind 2: 18-bit halfword match distance. For even distance,
; the two halves align with entire output words, so ordinary overlapping
; word copy is correct. Odd distance needs separate halfword handling.
 d6codec_v053_half:
        caie    16,2
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        jumpe   13,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        trne    13,1
        jrst    d6lz_fixed_base+(d6codec_v059_half_odd-d6lz_image_start)
        lsh     13,-1            ; even halfword distance -> words
        jrst    d6lz_fixed_base+(d6codec_v053_copy-d6lz_image_start)
; Odd halfword distance 2*k+1: output high half comes from the low
; half of word dst-k-1; output low half comes from the high half of
; word dst-k. Write the high half first, since with distance one the
; second source is the destination being constructed. No extra buffer.
d6codec_v059_half_odd:
        lsh     13,-1
        move    16,3
        sub     16,13
        soj     16,
        camge   16,5
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
d6codec_v059_half_odd_loop:
        hrrz    0,0(16)
        lsh     0,022
        movem   0,0(3)
        hlrz    0,1(16)
        iorm    0,0(3)
        aoj     16,
        aoj     3,
        sojg    12,d6lz_fixed_base+(d6codec_v059_half_odd_loop-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6codec_v053_next-d6lz_image_start)
 d6codec_v053_copy:
        jumpe   13,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        move    16,3
        sub     16,13
        camge   16,5
        jrst    d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
 d6codec_v053_copy_loop:
        move    0,0(16)
        movem   0,0(3)
        aoj     16,
        aoj     3,
        sojg    12,d6lz_fixed_base+(d6codec_v053_copy_loop-d6lz_image_start)
 d6codec_v053_next:
        lsh     10,1
        sojg    7,d6lz_fixed_base+(d6codec_v053_token-d6lz_image_start)
        camn    3,4
        jrst    d6lz_fixed_base+(d6codec_v053_finish-d6lz_image_start)
        jumpn   2,d6lz_fixed_base+(d6codec_v053_group-d6lz_image_start)
        movei   1,1                     ; whole-group boundary: refill
        popj    17,
d6codec_v061_no_input:
        camn    3,4
        jrst    d6lz_fixed_base+(d6codec_v053_finish-d6lz_image_start)
        movei   1,1
        popj    17,
 d6codec_v053_bad:
        seto    1,
        popj    17,
 d6codec_v053_finish:
        jumpn   2,d6lz_fixed_base+(d6codec_v053_bad-d6lz_image_start)
        setz    1,
        popj    17,
d6lz_image_end:
