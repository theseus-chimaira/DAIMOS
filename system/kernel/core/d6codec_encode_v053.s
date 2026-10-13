/* D6 codec v053 encoder. Staged only; not yet linked into KCORE. */
.text
.globl d6codec_v053_encode_literals
d6codec_v053_encode_literals:
        move    5,3
        push    17,5
        move    16,1
        jumpe   2,d6codec_v053_finish
 d6codec_v053_group_start:
        jumpe   4,d6codec_v053_fail
        move    5,3            ; group header location
        aoj     3,
        soj     4,
        setz    11,             ; match bitmap
        setz    10,            ; token count
        setz    7,             ; number of descriptors
 d6codec_v053_token:
        jumpe   2,d6codec_v053_group_end
        caige   10,036
        jrst    d6codec_v053_analyze
        jrst    d6codec_v053_group_end
 d6codec_v053_analyze:
        skipn   0(1)
        jrst    d6codec_v053_zero
        push    17,7
        push    17,11
        pushj   17,d6codec_v053_word_probe
        move    6,11
        pop     17,11
        pop     17,7
        caige   6,2
        jrst    d6codec_v053_literal
        jrst    d6codec_v053_match
 d6codec_v053_literal:
        jumpe   4,d6codec_v053_fail
        move    0,0(1)
        movem   0,0(3)
        aoj     3,
        aoj     1,
        soj     2,
        soj     4,
        aoja    10,d6codec_v053_token
 d6codec_v053_zero:
        movei   6,1
        movei   12,0177
        camle   12,2
        move    12,2
 d6codec_v053_zero_next:
        camge   6,12
        jrst    d6codec_v053_zero_check
        jrst    d6codec_v053_zero_done
 d6codec_v053_zero_check:
        move    14,1
        add     14,6
        skipn   0(14)
        aoja    6,d6codec_v053_zero_next
 d6codec_v053_zero_done:
        movei   13,0200000
 d6codec_v053_match:
        ; Insert descriptor at front of literals. At most 30 token words
        ; move per insertion; no extra storage beyond caller output.
        move    0,6
        lsh     0,11           ; length field at bit 9
        ior     0,13
        move    14,5
        aoj     14,
        move    12,7
        lsh     12,-1
        add     14,12          ; descriptor containing next half
        trne    7,1
        jrst    d6codec_v053_desc_right
        jumpe   4,d6codec_v053_fail
        move    12,3
 d6codec_v053_shift:
        camg    12,14
        jrst    d6codec_v053_shift_done
        soj     12,
        move    13,0(12)
        movem   13,1(12)
        jrst    d6codec_v053_shift
 d6codec_v053_shift_done:
        lsh     0,022
        movem   0,0(14)
        aoj     3,
        soj     4,
        jrst    d6codec_v053_flag
 d6codec_v053_desc_right:
        iorm    0,0(14)
 d6codec_v053_flag:
        move    0,[04000000000]
        move    12,10
        movn    12,12
        lsh     0,0(12)
        ior     11,0
        aoj     7,
        add     1,6
        sub     2,6
        aoja    10,d6codec_v053_token
 d6codec_v053_group_end:
        move    0,10
        lsh     0,036
        ior     0,11
        movem   0,0(5)
        jumpn   2,d6codec_v053_group_start
 d6codec_v053_finish:
        pop     17,5
        move    1,3
        sub     1,5
        popj    17,
 d6codec_v053_fail:
        pop     17,5
        seto    1,
        popj    17,

d6codec_v053_word_probe:
        ; Search backward up to 128 existing source words. AC15 holds
        ; candidate distance; AC13 holds the longest match distance.
        move    15,1
        sub     15,16
        jumpe   15,d6codec_v053_word_none
        caile   15,0200
        movei   15,0200
        setz    13,
        setz    11,                   ; best match length
        move    14,1
        sub     14,15                ; oldest candidate source pointer
 d6codec_v053_word_candidate:
        move    0,0(14)
        came    0,0(1)
        jrst    d6codec_v053_word_advance
        movei   6,1
        movei   7,0177
        camle   7,2
        move    7,2
 d6codec_v053_word_compare:
        camge   6,7
        jrst    d6codec_v053_word_compare_next
        jrst    d6codec_v053_word_consider
 d6codec_v053_word_compare_next:
        move    12,1
        add     12,6
        move    0,0(12)
        sub     12,15
        came    0,0(12)
        jrst    d6codec_v053_word_consider
        aoja    6,d6codec_v053_word_compare
 d6codec_v053_word_consider:
        camle   6,11
        jrst    d6codec_v053_word_new_best
        jrst    d6codec_v053_word_advance
 d6codec_v053_word_new_best:
        move    11,6
        move    13,15
        caie    11,0177
        jrst    d6codec_v053_word_advance
        jrst    d6codec_v053_word_ready
 d6codec_v053_word_advance:
        aoj     14,
        sojg    15,d6codec_v053_word_candidate
 d6codec_v053_word_ready:
        popj    17,
 d6codec_v053_word_none:
        setz    11,
        popj    17,

