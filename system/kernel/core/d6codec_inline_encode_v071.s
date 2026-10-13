/* D6 inline-token codec v071 encoder. Staged only; not yet linked into KCORE. */
.text
.globl d6codec_inline_encode_v071
d6codec_inline_encode_v071:
        move    5,3
        push    17,5
        move    16,1
        jumpe   2,d6codec_v071_finish
 d6codec_v071_group_start:
        jumpe   4,d6codec_v071_fail
        move    5,3            ; group header location
        aoj     3,
        soj     4,
        setz    11,             ; match bitmap
        setz    10,            ; token count
 d6codec_v071_token:
        jumpe   2,d6codec_v071_group_end
        caige   10,044
        jrst    d6codec_v071_analyze
        jrst    d6codec_v071_group_end
 d6codec_v071_analyze:
        skipn   0(1)
        jrst    d6codec_v071_zero
        push    17,11
        pushj   17,d6codec_v071_word_probe
        move    6,11
        pop     17,11
        caige   6,2
        jrst    d6codec_v071_literal
        jrst    d6codec_v071_match
 d6codec_v071_literal:
        jumpe   4,d6codec_v071_fail
        move    0,0(1)
        movem   0,0(3)
        aoj     3,
        aoj     1,
        soj     2,
        soj     4,
        aoja    10,d6codec_v071_token
 d6codec_v071_zero:
        movei   6,1
        movei   12,0177
        camle   12,2
        move    12,2
 d6codec_v071_zero_next:
        camge   6,12
        jrst    d6codec_v071_zero_check
        jrst    d6codec_v071_zero_done
 d6codec_v071_zero_check:
        move    14,1
        add     14,6
        skipn   0(14)
        aoja    6,d6codec_v071_zero_next
 d6codec_v071_zero_done:
        movei   13,0200000
 d6codec_v071_match:
        ; Emit descriptor inline instead of front-shifting paired halves.
        jumpe   4,d6codec_v071_fail
        move    0,6
        lsh     0,11
        ior     0,13
        movem   0,0(3)
        aoj     3,
        soj     4,
        move    0,[0400000000000]
        move    12,10
        movn    12,12
        lsh     0,0(12)
        ior     11,0
        add     1,6
        sub     2,6
        aoja    10,d6codec_v071_token
 d6codec_v071_group_end:
        movem   11,0(5)
        jumpn   2,d6codec_v071_group_start
 d6codec_v071_finish:
        pop     17,5
        move    1,3
        sub     1,5
        popj    17,
 d6codec_v071_fail:
        pop     17,5
        seto    1,
        popj    17,

d6codec_v071_word_probe:
        ; Search backward up to 128 existing source words. AC15 holds
        ; candidate distance; AC13 holds the longest match distance.
        move    15,1
        sub     15,16
        jumpe   15,d6codec_v071_word_none
        caile   15,0200
        movei   15,0200
        setz    13,
        setz    11,                   ; best match length
        move    14,1
        sub     14,15                ; oldest candidate source pointer
 d6codec_v071_word_candidate:
        move    0,0(14)
        came    0,0(1)
        jrst    d6codec_v071_word_advance
        movei   6,1
        movei   7,0177
        camle   7,2
        move    7,2
 d6codec_v071_word_compare:
        camge   6,7
        jrst    d6codec_v071_word_compare_next
        jrst    d6codec_v071_word_consider
 d6codec_v071_word_compare_next:
        move    12,1
        add     12,6
        move    0,0(12)
        sub     12,15
        came    0,0(12)
        jrst    d6codec_v071_word_consider
        aoja    6,d6codec_v071_word_compare
 d6codec_v071_word_consider:
        camle   6,11
        jrst    d6codec_v071_word_new_best
        jrst    d6codec_v071_word_advance
 d6codec_v071_word_new_best:
        move    11,6
        move    13,15
        caie    11,0177
        jrst    d6codec_v071_word_advance
        jrst    d6codec_v071_word_ready
 d6codec_v071_word_advance:
        aoj     14,
        sojg    15,d6codec_v071_word_candidate
 d6codec_v071_word_ready:
        popj    17,
 d6codec_v071_word_none:
        setz    11,
        popj    17,
