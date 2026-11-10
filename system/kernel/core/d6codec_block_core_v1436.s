/* Experimental PDP-6 block LZ/RLE optimized register core v1436.
 * NOT a production decoder. The exact wire format remains v1421.
 * The six-register refill state and error behavior are unchanged from v1424.
 * Reuse the output cursor as the BLT end address to avoid scratch setup,
 * and extract the match distance directly from the command left half.
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
        .globl d6codec_block_core_v1436

d6codec_block_core_v1436:
        jumpn   11,d6b_literal_v1436

d6b_command_v1436:
        jumpe   13,d6b_success_v1436
        jumpe   4,d6b_need_input_v1436
        soj     4,
        move    0,0(3)
        aoj     3,
        trne    0,0777777              ; reserved low 18 bits
        jrst    d6b_error_v1436
        move    6,0
        lsh     6,-033
        andi    6,0177
        aoj     6,                      ; length 1..128
        sub     13,6
        jumpl   13,d6b_error_v1436
        hlrz    7,0                     ; distance is LH low 9 bits
        andi    7,0777
        lsh     0,-042                 ; opcode bits 35..34
        jumpe   0,d6b_literal_header_v1436
        caie    0,1
        jrst    d6b_match_header_v1436
        jumpn   7,d6b_error_v1436
        setzm   0(12)
        caie    6,1
        jrst    d6b_zero_blt_v1436
        aoj     12,
        jrst    d6b_command_v1436

d6b_zero_blt_v1436:
        move    0,12
        hrl     0,12
        aoj     0,                      ; destination follows zero seed
        jrst    d6b_copy_blt_v1436

d6b_match_header_v1436:
        caie    0,2
        jrst    d6b_error_v1436
        aoj     7,                      ; distance 1..512
        move    5,12
        sub     5,7
        camge   5,14
        jrst    d6b_error_v1436
        move    0,5
        hrl     0,5
        hrr     0,12
d6b_copy_blt_v1436:
        add     12,6                    ; cursor = first unwritten word
        blt     0,-1(12)               ; source may overlap destination
        jrst    d6b_command_v1436

d6b_literal_header_v1436:
        jumpn   7,d6b_error_v1436
        move    11,6
d6b_literal_v1436:
        jumpe   4,d6b_need_input_v1436
        move    6,11
        camle   6,4
        move    6,4
        move    0,3
        hrl     0,3
        hrr     0,12
        add     12,6                   ; destination end is cursor - 1
        blt     0,-1(12)
        add     3,6
        sub     4,6
        sub     11,6
        jumpn   11,d6b_need_input_v1436
        jrst    d6b_command_v1436

d6b_need_input_v1436:
        movei   0,1
        popj    17,
d6b_error_v1436:
        seto    0,
        popj    17,
d6b_success_v1436:
        setz    0,
        popj    17,
