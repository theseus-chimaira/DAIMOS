/* Staged PDP-6 grouped D6CODEC resumable engine v1366, not linked to KCORE.
 *
 * AC1 points to a caller-owned 26-word state block (offsets below).
 * AC2 points to a disjoint input window of AC3 36-bit words.
 * AC0 returns 0 on exact success, +1 NEED_INPUT, -1 on failure.
 * AC1 remains the state pointer; AC2/AC3 are updated input cursor/count.
 * AC4..AC16 are volatile. AC17 temporarily holds a PUSHJ return word.
 * This is an assembly-only interface; production C ABI needs an adapter.
 *
 * Initialize state with phase 0, output cursor/end/base and compressed
 * remaining before the first call. DS_REMAIN=-1 selects trusted immutable
 * backing: decode until exact output completion, ignoring extra buffered
 * source words. All nonnegative DS_REMAIN values require exact consumption.
 * Output must remain unpublished on error.
 */
        .text
        .globl d6codec_stream_v1366
        .equ DS_PHASE,0
        .equ DS_DST,1
        .equ DS_END,2
        .equ DS_BASE,3
        .equ DS_REMAIN,4
        .equ DS_FLAGS,5
        .equ DS_TOKENS,6
        .equ DS_MATCHES,7
        .equ DS_DESC_NEED,010
        .equ DS_DESC_HAVE,011
        .equ DS_DESC_INDEX,012
        .equ DS_DESCRIPTORS,013
        .equ DS_WORDS,032

d6codec_stream_v1366:
        ; Validate the complete persistent state and output geometry.
        ; Output end is exclusive and may equal physical 01000000.
        move    0,1
        lsh     0,-022
        jumpn   0,ds_bad
        move    0,1
        addi    0,DS_WORDS
        camle   0,[01000000]
        jrst    ds_bad
        move    4,DS_BASE(1)
        move    5,DS_DST(1)
        move    6,DS_END(1)
        jumpl   4,ds_bad
        jumpl   5,ds_bad
        jumpl   6,ds_bad
        camle   6,[01000000]
        jrst    ds_bad
        camle   4,5
        jrst    ds_bad
        camle   5,6
        jrst    ds_bad
        move    7,DS_REMAIN(1)
        camn    7,[-1]
        jrst    ds_state_check
        jumpl   7,ds_bad
        lsh     7,-022
        jumpn   7,ds_bad
ds_state_check:
        ; State memory must never be part of the output image.
        camge   1,6
        jrst    ds_state_overlap
        jrst    ds_source_check
ds_state_overlap:
        camge   4,0
        jrst    ds_bad
ds_source_check:
        ; Reject nonphysical source windows before consuming any input.
        move    0,2
        lsh     0,-022
        jumpn   0,ds_bad
        move    0,3
        lsh     0,-022
        jumpn   0,ds_bad
        move    0,2
        add     0,3
        camle   0,[01000000]
        jrst    ds_bad
        ; Source must not overlap the complete output history/range.
        jumpe   3,ds_main
        move    4,DS_BASE(1)
        move    5,DS_END(1)
        camge   2,5
        jrst    ds_overlap
        jrst    ds_state_input_check
ds_overlap:
        camge   4,0
        jrst    ds_bad
ds_state_input_check:
        ; Refills must not overwrite the persistent decoder state.
        move    4,1
        addi    4,DS_WORDS
        camge   2,4
        jrst    ds_state_input_overlap
        jrst    ds_main
ds_state_input_overlap:
        camge   1,0
        jrst    ds_bad
ds_main:
        move    4,DS_PHASE(1)
        jumpe   4,ds_control
        caie    4,1
        jrst    ds_other
        jrst    ds_descriptor
ds_other:
        caie    4,2
        jrst    ds_other2
        jrst    ds_token
ds_other2:
        caie    4,3
        jrst    ds_other3
        jrst    ds_literal
ds_other3:
        caie    4,4
        jrst    ds_bad
        jrst    ds_finish

ds_control:
        move    4,DS_DST(1)
        camn    4,DS_END(1)
        jrst    ds_finish
        pushj   17,ds_getword
        jumpn   4,ds_get_return
        move    5,0
        lsh     5,-036
        jumpe   5,ds_bad
        caile   5,036
        jrst    ds_bad
        move    6,0
        lsh     6,6
        ; After shifting count flags to the high bits, the unused
        ; low flags must be zero. Check by left shifting count bits.
        move    7,6
        move    10,5
ds_validate_flags:
        lsh     7,1
        sojg    10,ds_validate_flags
        jumpn   7,ds_bad
        movem   6,DS_FLAGS(1)
        movem   5,DS_TOKENS(1)
        setz    7,
        move    10,5
        move    11,6
ds_count_matches:
        skipge  11
        aoj     7,
        lsh     11,1
        sojg    10,ds_count_matches
        movem   7,DS_MATCHES(1)
        aoj     7,
        lsh     7,-1
        movem   7,DS_DESC_NEED(1)
        setzm   DS_DESC_HAVE(1)
        setzm   DS_DESC_INDEX(1)
        jumpe   7,ds_prepare_token
        movei   4,1
        movem   4,DS_PHASE(1)
        jrst    ds_descriptor

ds_descriptor:
        pushj   17,ds_getword
        jumpn   4,ds_get_return
        move    5,DS_DESC_HAVE(1)
        move    6,1
        addi    6,DS_DESCRIPTORS
        add     6,5
        movem   0,0(6)
        aoj     5,
        movem   5,DS_DESC_HAVE(1)
        came    5,DS_DESC_NEED(1)
        jrst    ds_descriptor
        move    5,DS_MATCHES(1)
        trnn    5,1
        jrst    ds_prepare_token
        trne    0,0777777
        jrst    ds_bad
ds_prepare_token:
        movei   4,2
        movem   4,DS_PHASE(1)
        jrst    ds_token

ds_token:
        move    5,DS_TOKENS(1)
        jumpe   5,ds_group_end
        move    6,DS_FLAGS(1)
        jumpl   6,ds_match
        movei   4,3
        movem   4,DS_PHASE(1)
        jrst    ds_literal

ds_literal:
        move    5,DS_DST(1)
        camge   5,DS_END(1)
        jrst    ds_literal_room
        jrst    ds_bad
ds_literal_room:
        pushj   17,ds_getword
        jumpn   4,ds_get_return
        movem   0,0(5)
        aoj     5,
        movem   5,DS_DST(1)
        jrst    ds_token_done

ds_match:
        move    5,DS_DESC_INDEX(1)
        move    6,5
        lsh     6,-1
        addi    6,DS_DESCRIPTORS
        add     6,1
        move    0,0(6)
        trne    5,1
        jrst    ds_desc_right
        hlrz    0,0
        jrst    ds_desc_ready
ds_desc_right:
        hrrz    0,0
ds_desc_ready:
        aoj     5,
        movem   5,DS_DESC_INDEX(1)
        move    7,0
        lsh     7,-11
        andi    7,0177
        jumpe   7,ds_bad
        move    10,DS_DST(1)
        move    11,10
        add     11,7
        camle   11,DS_END(1)
        jrst    ds_bad
        move    12,0
        andi    12,0777
        lsh     0,-020
        jumpe   0,ds_word_copy
        caie    0,1
        jrst    ds_half_kind
        jumpn   12,ds_bad
ds_zero_loop:
        setzm   0(10)
        aoj     10,
        sojg    7,ds_zero_loop
        jrst    ds_copy_done
ds_half_kind:
        caie    0,2
        jrst    ds_bad
        trne    12,1
        jrst    ds_bad
        lsh     12,-1
ds_word_copy:
        jumpe   12,ds_bad
        move    13,10
        sub     13,12
        camge   13,DS_BASE(1)
        jrst    ds_bad
ds_copy_loop:
        move    0,0(13)
        movem   0,0(10)
        aoj     13,
        aoj     10,
        sojg    7,ds_copy_loop
ds_copy_done:
        movem   10,DS_DST(1)
ds_token_done:
        move    5,DS_FLAGS(1)
        lsh     5,1
        movem   5,DS_FLAGS(1)
        sos     DS_TOKENS(1)
        movei   4,2
        movem   4,DS_PHASE(1)
        jrst    ds_token

ds_group_end:
        move    5,DS_FLAGS(1)
        jumpn   5,ds_bad
        move    5,DS_DST(1)
        camn    5,DS_END(1)
        jrst    ds_mark_done
        movei   4,0
        movem   4,DS_PHASE(1)
        jrst    ds_control
ds_mark_done:
        movei   4,4
        movem   4,DS_PHASE(1)
ds_finish:
        move    4,DS_REMAIN(1)
        camn    4,[-1]
        jrst    ds_succeed
        jumpn   4,ds_bad
ds_done:
        jumpn   3,ds_bad
ds_succeed:
        setz    0,
        popj    17,

/* Get one compressed word; never consume beyond the declared total.
 * 4=0 success, 4=1 NEED_INPUT, 4=-1 malformed count.
 * DS_REMAIN=-1 never decrements; it is the trusted immutable mode.
 * The caller saves its own return PC on AC17 (one stack word only).
 */
ds_getword:
        move    4,DS_REMAIN(1)
        camn    4,[-1]
        jrst    ds_get_trusted
        jumple  4,ds_get_bad
        jumpe   3,ds_get_empty
        move    0,0(2)
        aoj     2,
        soj     3,
        sos     DS_REMAIN(1)
        setz    4,
        popj    17,
ds_get_trusted:
        jumpe   3,ds_get_empty
        move    0,0(2)
        aoj     2,
        soj     3,
        setz    4,
        popj    17,
ds_get_empty:
        movei   4,1
        popj    17,
ds_get_bad:
        seto    4,
        popj    17,
ds_get_return:
        move    0,4
        popj    17,
ds_bad:
        seto    0,
        popj    17,
