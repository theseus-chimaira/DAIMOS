/* D6 codec v058 fixed-address experimental group-atomic resumable PDP-6 core.
 * ABI identical to legacy 000060 decoder: AC3,4,10..14 and AC0 result.
 * All internal branches target the installed 000060 image, not this source.
 * No static storage and no stack use. Not yet production-linked.
 * The caller must keep incomplete group prefixes for refill.
 */
        .text
        .equ d6codec_fixed_base,000060
        .globl d6codec_resumable_v058
        .globl d6codec_image_start
        .globl d6codec_image_end
d6codec_resumable_v058:
d6codec_image_start:
        jumpe   13,d6codec_fixed_base+(d6r058_done-d6codec_image_start)
 d6r058_group:
        jumpe   4,d6codec_fixed_base+(d6r058_need-d6codec_image_start)
        move    0,0(3)                 ; grouped control
        move    1,0
        lsh     1,-036                 ; token count (1..036)
        jumpe   1,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        caile   1,036
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    6,0
        andi    6,077                  ; reserved low control bits
        jumpn   6,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    10,0
        lsh     10,6                   ; token flags start at sign bit
        move    6,10
        move    7,1
        setz    5,                     ; descriptor count
 d6r058_population:
        skipge  6
        aoj     5,
        lsh     6,1
        sojg    7,d6codec_fixed_base+(d6r058_population-d6codec_image_start)
        jumpn   6,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    2,5
        aoj     2,
        lsh     2,-1                   ; descriptor word count
        move    7,1
        sub     7,5                    ; literal count
        add     7,2
        aoj     7,                     ; full group size
        caile   7,037
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        camle   7,4
        jrst    d6codec_fixed_base+(d6r058_need-d6codec_image_start)
        trne    5,1                    ; odd descriptors: unused RH zero
        jrst    d6codec_fixed_base+(d6r058_padding-d6codec_image_start)
        jrst    d6codec_fixed_base+(d6r058_group_ready-d6codec_image_start)
 d6r058_padding:
        move    6,3
        add     6,2
        hrrz    6,0(6)
        jumpn   6,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
 d6r058_group_ready:
        move    6,3
        aoj     6,                     ; descriptor cursor
        sub     4,7                    ; count remaining after group
        add     3,7                    ; source now points past group
        move    7,6
        add     7,2                    ; literal cursor
        move    11,1                   ; token count
        setz    5,                     ; descriptor-half toggle
 d6r058_token:
        jumpl   10,d6codec_fixed_base+(d6r058_match-d6codec_image_start)
        jumpe   13,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    0,0(7)
        movem   0,0(12)
        aoj     7,
        aoj     12,
        soj     13,
        jrst    d6codec_fixed_base+(d6r058_next-d6codec_image_start)
 d6r058_match:
        move    0,0(6)
        jumpe   5,d6codec_fixed_base+(d6r058_desc_left-d6codec_image_start)
        hrrz    0,0
        aoj     6,
        setz    5,
        jrst    d6codec_fixed_base+(d6r058_desc-d6codec_image_start)
 d6r058_desc_left:
        hlrz    0,0
        movei   5,1
 d6r058_desc:
        move    2,0
        lsh     2,-11                  ; length
        andi    2,0177
        jumpe   2,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        camle   2,13
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    1,0
        lsh     1,-020                 ; kind
        andi    0,0777                ; distance
        jumpe   1,d6codec_fixed_base+(d6r058_copy-d6codec_image_start)
        caie    1,1
        jrst    d6codec_fixed_base+(d6r058_half-d6codec_image_start)
        jumpn   0,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
 d6r058_zero:
        setzm   0(12)
        aoj     12,
        soj     13,
        sojg    2,d6codec_fixed_base+(d6r058_zero-d6codec_image_start)
        jrst    d6codec_fixed_base+(d6r058_next-d6codec_image_start)
 d6r058_half:
        caie    1,2
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        jumpe   0,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        trne    0,1
        jrst    d6codec_fixed_base+(d6r058_odd-d6codec_image_start)
        lsh     0,-1
        jrst    d6codec_fixed_base+(d6r058_copy-d6codec_image_start)
 d6r058_odd:
        aoj     0,
        lsh     0,-1
        move    1,12
        sub     1,0
        camge   1,14
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
 d6r058_odd_copy:
        hrrz    0,0(1)
        lsh     0,022
        movem   0,0(12)
        hlrz    0,1(1)
        iorm    0,0(12)
        aoj     1,
        aoj     12,
        soj     13,
        sojg    2,d6codec_fixed_base+(d6r058_odd_copy-d6codec_image_start)
        jrst    d6codec_fixed_base+(d6r058_next-d6codec_image_start)
 d6r058_copy:
        jumpe   0,d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
        move    1,12
        sub     1,0
        camge   1,14
        jrst    d6codec_fixed_base+(d6r058_bad-d6codec_image_start)
 d6r058_copy_loop:
        move    0,0(1)
        movem   0,0(12)
        aoj     1,
        aoj     12,
        soj     13,
        sojg    2,d6codec_fixed_base+(d6r058_copy_loop-d6codec_image_start)
 d6r058_next:
        lsh     10,1
        sojg    11,d6codec_fixed_base+(d6r058_token-d6codec_image_start)
        setz    10,
        setz    11,
        jumpe   13,d6codec_fixed_base+(d6r058_done-d6codec_image_start)
        jrst    d6codec_fixed_base+(d6r058_group-d6codec_image_start)
 d6r058_need:
        setz    10,
        setz    11,
        movei   0,1
        popj    17,
 d6r058_bad:
        seto    0,
        popj    17,
 d6r058_done:
        setz    0,
        popj    17,
d6codec_image_end:
