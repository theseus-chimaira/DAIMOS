/* Staged PDP-6 grouped D6CODEC fast-path decoder v1420.
 * NOT linked to production KCORE. Wire format and ABI unchanged.
 * AC14/AC15 cache token flags/count between refills; on NEED_INPUT,
 * the literal path spills them into continuation state. Long zero
 * runs, overlapping word copies, and complete literal-only groups
 * use BLT with bounded, disjoint input/output regions.
 *
 * AC1 points to a caller-owned 25-word state block (offsets below).
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
        .globl d6codec_stream_v1420
        .equ DS_PHASE,0
        .equ DS_DST,1
        .equ DS_END,2
        .equ DS_BASE,3
        .equ DS_REMAIN,4
        .equ DS_FLAGS,5
        .equ DS_TOKENS,6
        .equ DS_MATCHES,7
        .equ DS_DESC_NEED,010
        ; Descriptor ingestion and token traversal never run together.
        ; Reuse one counter in both phases instead of retaining two.
        .equ DS_DESC_INDEX,011
        .equ DS_DESCRIPTORS,012
        .equ DS_WORDS,031

d6codec_stream_v1420:
        ; Validate the complete persistent state and output geometry.
        ; Output end is exclusive and may equal physical 01000000.
        tlne    1,0777777              ; 18-bit physical state pointer
        jrst    ds_bad
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
        tlne    2,0777777
        jrst    ds_bad
        tlne    3,0777777
        jrst    ds_bad
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
        cain    4,1
        jrst    ds_descriptor
        cain    4,2
        jrst    ds_token
        cain    4,3
        jrst    ds_resume_literal
        cain    4,4
        jrst    ds_finish
        jrst    ds_bad

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
        setzm   DS_DESC_INDEX(1)
        jumpe   7,ds_prepare_token
        movei   4,1
        movem   4,DS_PHASE(1)
        jrst    ds_descriptor

ds_descriptor:
        pushj   17,ds_getword
        jumpn   4,ds_get_return
        move    5,DS_DESC_INDEX(1)
        move    6,1
        add     6,5
        movem   0,DS_DESCRIPTORS(6)
        aoj     5,
        movem   5,DS_DESC_INDEX(1)
        came    5,DS_DESC_NEED(1)
        jrst    ds_descriptor
        setzm   DS_DESC_INDEX(1)        ; ingestion -> token cursor
        move    5,DS_MATCHES(1)
        trnn    5,1
        jrst    ds_prepare_token
        trne    0,0777777
        jrst    ds_bad
ds_prepare_token:
        ; Literal-only groups have no descriptor table. Use the
        ; existing streaming state machine when the input window
        ; cannot contain the whole group.
        skipn   DS_MATCHES(1)
        jrst    ds_fast_literal
ds_set_token:
        movei   4,2
        movem   4,DS_PHASE(1)
        move    14,DS_FLAGS(1)
        move    15,DS_TOKENS(1)
        jrst    ds_token

ds_token:
        jumpe   15,ds_group_end
        jumpl   14,ds_match
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

ds_resume_literal:
        move    14,DS_FLAGS(1)
        move    15,DS_TOKENS(1)
        jrst    ds_literal

ds_fast_literal:
        move    5,DS_TOKENS(1)
        camle   5,3
        jrst    ds_set_token
        move    6,DS_REMAIN(1)
        jumpl   6,ds_fast_room
        camle   5,6
        jrst    ds_bad
ds_fast_room:
        move    6,DS_DST(1)
        move    7,6
        add     7,5
        camle   7,DS_END(1)
        jrst    ds_bad
        ; Source input window is disjoint from output and has been
        ; geometry-checked at entry. A nonempty literal group needs
        ; only a single PDP-6 block transfer.
        move    0,2
        hrl     0,2
        hrr     0,6
        move    4,7
        soj     4,
        blt     0,0(4)
        add     2,5
        sub     3,5
        movem   7,DS_DST(1)
        move    6,DS_REMAIN(1)
        jumpl   6,ds_fast_next
        sub     6,5
        movem   6,DS_REMAIN(1)
ds_fast_next:
        camn    7,DS_END(1)
        jrst    ds_mark_done
        setzm   DS_PHASE(1)
        jrst    ds_control

ds_match:
        move    5,DS_DESC_INDEX(1)
        move    6,5
        lsh     6,-1
        add     6,1
        move    0,DS_DESCRIPTORS(6)
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
        caie    7,1
        jrst    ds_zero_blt
        aoj     10,
        jrst    ds_copy_done
ds_zero_blt:
        move    0,10
        hrl     0,10
        aoj     0,
        move    5,11
        soj     5,
        blt     0,0(5)
        move    10,11
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
        move    0,13
        hrl     0,13
        hrr     0,10
        move    5,11
        soj     5,
        blt     0,0(5)
        move    10,11
ds_copy_done:
        movem   10,DS_DST(1)
ds_token_done:
        lsh     14,1
        soj     15,
        movei   4,2
        movem   4,DS_PHASE(1)
        jrst    ds_token

ds_group_end:
        jumpn   14,ds_bad
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
        jrst    ds_get_input
        jumple  4,ds_get_bad
        jumpe   3,ds_get_empty
        sos     DS_REMAIN(1)
ds_get_input:
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
        ; Only a literal can suspend with an active token pair.
        move    5,DS_PHASE(1)
        caie    5,3
        popj    17,
        movem   14,DS_FLAGS(1)
        movem   15,DS_TOKENS(1)
        popj    17,
ds_bad:
        seto    0,
        popj    17,
