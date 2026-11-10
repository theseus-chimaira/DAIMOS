/**
 * @file block_decompressor_v1452.inc
 * @brief Fixed-low-core copy of the validated block v1441 codec.
 *
 * All internal jumps target d6b_low_base_v1452 plus the offset of
 * their local label relative to d6b_image_start_v1452.  Stage-1 may
 * copy this compiled image from any location to executive address 000060
 * and execute it independently. The identical source is linked as the
 * first KCORE TEXT object at address 000060. Its compressed wire format
 * and six-register streaming state are unchanged from block v1441.
 *
 * Keep in exact instruction parity with d6codec_block_core_v1441.s;
 * testkit verifies the full 60-word image against the canonical
 * independently assembled v1441 object before boot activation.
 */
        .equ d6b_low_base_v1452,000060
        .text
        .globl d6codec_block_fixed_v1452
        .globl d6codec_block_core_v1452
        .globl d6b_image_start_v1452
        .globl d6b_image_end_v1452
d6codec_block_fixed_v1452:
d6codec_block_core_v1452:
d6b_image_start_v1452:
        jumpn   11,d6b_low_base_v1452+(d6b_literal_v1452-d6b_image_start_v1452)

d6b_command_v1452:
        jumpe   13,d6b_low_base_v1452+(d6b_success_v1452-d6b_image_start_v1452)
        jumpe   4,d6b_low_base_v1452+(d6b_need_input_v1452-d6b_image_start_v1452)
        soj     4,
        move    0,0(3)
        aoj     3,
        trne    0,0777777              ; reserved low 18 bits
        jrst    d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        move    6,0
        lsh     6,-033
        andi    6,0177
        aoj     6,                      ; length 1..128
        sub     13,6
        jumpl   13,d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        hlrz    7,0                     ; distance is LH low 9 bits
        andi    7,0777
        lsh     0,-042                 ; opcode bits 35..34
        jumpe   0,d6b_low_base_v1452+(d6b_literal_header_v1452-d6b_image_start_v1452)
        caie    0,1
        jrst    d6b_low_base_v1452+(d6b_match_header_v1452-d6b_image_start_v1452)
        jumpn   7,d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        setzm   0(12)
        caie    6,1
        jrst    d6b_low_base_v1452+(d6b_zero_blt_v1452-d6b_image_start_v1452)
        aoja    12,d6b_low_base_v1452+(d6b_command_v1452-d6b_image_start_v1452)    ; one-word zero run

d6b_zero_blt_v1452:
        hrlz    0,12                    ; source = zero seed
        hrri    0,1(12)                 ; destination follows seed
        jrst    d6b_low_base_v1452+(d6b_copy_blt_v1452-d6b_image_start_v1452)

d6b_match_header_v1452:
        caie    0,2
        jrst    d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        aoj     7,                      ; distance 1..512
        move    5,12
        sub     5,7
        camge   5,14
        jrst    d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        hrlz    0,5                     ; source = validated match start
        hrr     0,12                    ; destination = output cursor
d6b_copy_blt_v1452:
        add     12,6                    ; cursor = first unwritten word
        blt     0,-1(12)               ; source may overlap destination
        jrst    d6b_low_base_v1452+(d6b_command_v1452-d6b_image_start_v1452)

d6b_literal_header_v1452:
        jumpn   7,d6b_low_base_v1452+(d6b_error_v1452-d6b_image_start_v1452)
        move    11,6
d6b_literal_v1452:
        jumpe   4,d6b_low_base_v1452+(d6b_need_input_v1452-d6b_image_start_v1452)
        move    6,11
        camle   6,4
        move    6,4
        hrlz    0,3                     ; source = compressed literal
        hrr     0,12                    ; destination = output cursor
        add     12,6                   ; destination end is cursor - 1
        blt     0,-1(12)
        add     3,6
        sub     4,6
        sub     11,6
        jumpn   11,d6b_low_base_v1452+(d6b_need_input_v1452-d6b_image_start_v1452)
        jrst    d6b_low_base_v1452+(d6b_command_v1452-d6b_image_start_v1452)

d6b_need_input_v1452:
        movei   0,1
        popj    17,
d6b_error_v1452:
        seto    0,
        popj    17,
d6b_success_v1452:
        setz    0,
        popj    17,
d6b_image_end_v1452:
