/* Experimental 36-bit block LZ/RLE streaming decoder v1421.
 * NOT a production format. The host producer in daimos-testkit owns
 * the experimental stream; the grouped v1420 format is unchanged.
 *
 * Command:
 *   bits 35..34: 0 literal, 1 zero run, 2 backward word copy, 3 invalid
 *   bits 33..27: output length minus one (1..128)
 *   bits 26..18: match distance minus one (1..512); zero otherwise
 *   bits 17..0:  zero, reserved
 * A literal command is followed by exactly length 36-bit words.
 *
 * AC1 state, AC2 input window, AC3 available input words.
 * AC0 = 0 success, +1 NEED_INPUT, -1 error; AC2/AC3 advanced.
 * AC4..AC16 volatile. State: six words, no permanent data/BSS.
 * All input and output physical address ranges must be disjoint.
 */
        .text
        .globl d6codec_block_stream_v1421
        .equ BK_PHASE,0
        .equ BK_DST,1
        .equ BK_END,2
        .equ BK_BASE,3
        .equ BK_REMAIN,4
        .equ BK_PENDING,5
        .equ BK_WORDS,6

d6codec_block_stream_v1421:
        tlne    1,0777777
        jrst    bk_bad
        move    0,1
        addi    0,BK_WORDS
        camle   0,[01000000]
        jrst    bk_bad
        move    4,BK_BASE(1)
        move    5,BK_DST(1)
        move    6,BK_END(1)
        jumpl   4,bk_bad
        jumpl   5,bk_bad
        jumpl   6,bk_bad
        camle   6,[01000000]
        jrst    bk_bad
        camle   4,5
        jrst    bk_bad
        camle   5,6
        jrst    bk_bad
        move    7,BK_REMAIN(1)
        camn    7,[-1]
        jrst    bk_state_checked
        jumpl   7,bk_bad
        tlne    7,0777777
        jrst    bk_bad
bk_state_checked:
        camge   1,6
        jrst    bk_state_overlap
        jrst    bk_input_check
bk_state_overlap:
        camge   4,0
        jrst    bk_bad
bk_input_check:
        tlne    2,0777777
        jrst    bk_bad
        tlne    3,0777777
        jrst    bk_bad
        move    0,2
        add     0,3
        camle   0,[01000000]
        jrst    bk_bad
        jumpe   3,bk_state_input_check
        camge   2,6
        jrst    bk_input_overlap
        jrst    bk_state_input_check
bk_input_overlap:
        camge   4,0
        jrst    bk_bad
bk_state_input_check:
        jumpe   3,bk_dispatch
        move    4,1
        addi    4,BK_WORDS
        camge   2,4
        jrst    bk_state_input_overlap
        jrst    bk_dispatch
bk_state_input_overlap:
        camge   1,0
        jrst    bk_bad
bk_dispatch:
        move    4,BK_PHASE(1)
        jumpe   4,bk_command
        caie    4,1
        jrst    bk_done_check
        jrst    bk_literal
bk_done_check:
        caie    4,2
        jrst    bk_bad
        jrst    bk_finish

bk_command:
        move    5,BK_DST(1)
        camn    5,BK_END(1)
        jrst    bk_mark_done
        pushj   17,bk_getword
        jumpn   4,bk_get_return
        ; Command length and reserved fields.
        move    5,0
        andi    5,0777777
        jumpn   5,bk_bad
        move    5,0
        lsh     5,-033
        andi    5,0177
        aoj     5,
        move    6,BK_DST(1)
        move    11,6
        add     11,5
        camle   11,BK_END(1)
        jrst    bk_bad
        move    7,0
        lsh     7,-022
        andi    7,0777
        move    10,0
        lsh     10,-042                 ; high two bits (34 decimal)
        jumpe   10,bk_literal_command
        caie    10,1
        jrst    bk_copy_command
        jumpn   7,bk_bad
        ; Set one zero; propagate it through the rest of the run.
        setzm   0(6)
        caie    5,1
        jrst    bk_zero_blt
        jrst    bk_block_done
bk_zero_blt:
        move    0,6
        hrl     0,6
        aoj     0,
        move    12,11
        soj     12,
        blt     0,0(12)
        jrst    bk_block_done

bk_copy_command:
        caie    10,2
        jrst    bk_bad
        aoj     7,                      ; distance 1..512
        move    12,6
        sub     12,7
        camge   12,BK_BASE(1)
        jrst    bk_bad
        move    0,12
        hrl     0,12
        hrr     0,6
        move    12,11
        soj     12,
        blt     0,0(12)
        jrst    bk_block_done

bk_literal_command:
        jumpn   7,bk_bad
        movem   5,BK_PENDING(1)
        movei   4,1
        movem   4,BK_PHASE(1)
        jrst    bk_literal

bk_literal:
        move    5,BK_PENDING(1)
        jumpe   5,bk_bad
        camle   5,3
        move    5,3
        jumpe   5,bk_need_input
        move    7,BK_REMAIN(1)
        jumpl   7,bk_literal_room
        camle   5,7
        jrst    bk_bad
bk_literal_room:
        move    6,BK_DST(1)
        move    11,6
        add     11,5
        ; Entire block bounds were checked at command time.
        move    0,2
        hrl     0,2
        hrr     0,6
        move    12,11
        soj     12,
        blt     0,0(12)
        add     2,5
        sub     3,5
        movem   11,BK_DST(1)
        movn    5,5
        addm    5,BK_PENDING(1)
        move    7,BK_REMAIN(1)
        jumpl   7,bk_literal_next
        add     7,5
        movem   7,BK_REMAIN(1)
bk_literal_next:
        skipn   BK_PENDING(1)
        jrst    bk_literal_finished
        jrst    bk_need_input
bk_literal_finished:
        setzm   BK_PHASE(1)
        jrst    bk_command

bk_block_done:
        movem   11,BK_DST(1)
        jrst    bk_command

bk_mark_done:
        movei   4,2
        movem   4,BK_PHASE(1)
bk_finish:
        move    4,BK_REMAIN(1)
        camn    4,[-1]
        jrst    bk_success
        jumpn   4,bk_bad
        jumpn   3,bk_bad
bk_success:
        setz    0,
        popj    17,

bk_getword:
        move    4,BK_REMAIN(1)
        camn    4,[-1]
        jrst    bk_get_input
        jumple  4,bk_get_bad
        jumpe   3,bk_get_empty
        sos     BK_REMAIN(1)
bk_get_input:
        jumpe   3,bk_get_empty
        move    0,0(2)
        aoj     2,
        soj     3,
        setz    4,
        popj    17,
bk_get_empty:
        movei   4,1
        popj    17,
bk_get_bad:
        seto    4,
        popj    17,
bk_get_return:
        move    0,4
        popj    17,
bk_need_input:
        move    4,BK_REMAIN(1)
        jumpe   4,bk_bad
        movei   0,1
        popj    17,
bk_bad:
        seto    0,
        popj    17,
