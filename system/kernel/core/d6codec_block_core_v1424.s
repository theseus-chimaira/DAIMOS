/* Experimental PDP-6 block LZ/RLE register-resident core v1424.
 * NOT a production decoder. The exact wire format is v1421.
 *
 * Streaming ABI mirrors the fixed D6LZ36 low-core machine:
 *   AC3  next compressed input word
 *   AC4  compressed words available in this refill
 *   AC11 literal words pending (zero means fetch next command)
 *   AC12 output cursor
 *   AC13 output words remaining
 *   AC14 output origin (backreference lower bound)
 *   AC0  result: 0 complete, +1 NEED_INPUT, -1 failure
 *
 * AC11..AC14 retain their meaning across NEED_INPUT; all other
 * scratch ACs used here are caller-volatile. No heap, BSS, or state
 * block. The ABI adapter owns exact compressed counts and geometry.
 */
        .text
        .globl d6codec_block_core_v1424

d6codec_block_core_v1424:
        jumpn   11,d6b_literal_v1424

d6b_command_v1424:
        jumpe   13,d6b_success_v1424
        jumpe   4,d6b_need_input_v1424
        soj     4,
        move    0,0(3)
        aoj     3,
        move    5,0
        andi    5,0777777              ; reserved low 18 bits
        jumpn   5,d6b_error_v1424
        move    6,0
        lsh     6,-033
        andi    6,0177
        aoj     6,                      ; length 1..128
        sub     13,6
        jumpl   13,d6b_error_v1424
        move    7,0
        lsh     7,-022
        andi    7,0777
        lsh     0,-042                 ; opcode bits 35..34
        jumpe   0,d6b_literal_header_v1424
        caie    0,1
        jrst    d6b_match_header_v1424
        jumpn   7,d6b_error_v1424
        setzm   0(12)
        caie    6,1
        jrst    d6b_zero_blt_v1424
        aoj     12,
        jrst    d6b_command_v1424

d6b_zero_blt_v1424:
        move    0,12
        hrl     0,12
        aoj     0,
        move    5,12
        add     5,6
        soj     5,
        blt     0,0(5)
        add     12,6
        jrst    d6b_command_v1424

d6b_match_header_v1424:
        caie    0,2
        jrst    d6b_error_v1424
        aoj     7,                      ; distance 1..512
        move    5,12
        sub     5,7
        camge   5,14
        jrst    d6b_error_v1424
        move    0,5
        hrl     0,5
        hrr     0,12
        move    5,12
        add     5,6
        soj     5,
        blt     0,0(5)
        add     12,6
        jrst    d6b_command_v1424

d6b_literal_header_v1424:
        jumpn   7,d6b_error_v1424
        move    11,6
d6b_literal_v1424:
        jumpe   4,d6b_need_input_v1424
        move    6,11
        camle   6,4
        move    6,4
        move    0,3
        hrl     0,3
        hrr     0,12
        move    5,12
        add     5,6
        soj     5,
        blt     0,0(5)
        add     3,6
        sub     4,6
        add     12,6
        sub     11,6
        jumpn   11,d6b_need_input_v1424
        jrst    d6b_command_v1424

d6b_need_input_v1424:
        movei   0,1
        popj    17,
d6b_error_v1424:
        seto    0,
        popj    17,
d6b_success_v1424:
        setz    0,
        popj    17,
