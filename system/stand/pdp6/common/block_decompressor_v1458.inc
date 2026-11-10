/**
 * @file block_decompressor_v1458.inc
 * @brief Shared fixed-low-core 53-word block decoder for Stage-1 and KCORE.
 *
 * All internal jumps target d6b_low_base_v1458 plus the offset of
 * their local label relative to d6b_image_start_v1458.  Stage-1 may
 * copy this compiled image from any location to executive address 000060
 * and execute it independently. The identical source is linked as the
 * first KCORE TEXT object at address 000060. Its compressed wire format
 * and six-register streaming state are unchanged from block v1441.
 *
 * Compared with the original 61-instruction v1452 fixed image, this
 * combines the input count check with SOJL, inverts the final literal
 * branch, uses CAIN/AOJA for one-word zero runs, extracts the length
 * with LDB from an in-image byte pointer, and constructs the backward
 * match source with indexed MOVNI. Result: 52 executable instructions
 * plus one byte-pointer word, zero static DATA/BSS and relocations.
 *
 * Preserve all file-data validation here. The VFS adapter verifies
 * compressed extent consumption, input refill and available buffers.
 * Both executable code and the inline byte pointer are copied to 060.
 */
        .equ d6b_low_base_v1458,000060
        .text
        .globl d6codec_block_fixed_v1458
        .globl d6codec_block_core_v1458
        .globl d6b_image_start_v1458
        .globl d6b_image_end_v1458
d6codec_block_fixed_v1458:
d6codec_block_core_v1458:
d6b_image_start_v1458:
        jumpn   11,d6b_low_base_v1458+(d6b_literal_v1458-d6b_image_start_v1458)

d6b_command_v1458:
        jumpe   13,d6b_low_base_v1458+(d6b_success_v1458-d6b_image_start_v1458)
        sojl    4,d6b_low_base_v1458+(d6b_need_input_v1458-d6b_image_start_v1458)
        move    0,0(3)
        aoj     3,
        trne    0,0777777              ; reserved low 18 bits
        jrst    d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        ldb     6,d6b_low_base_v1458+(d6b_length_pointer_v1458-d6b_image_start_v1458)
        aoj     6,                      ; length 1..128
        sub     13,6
        jumpl   13,d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        hlrz    7,0                     ; distance is LH low 9 bits
        andi    7,0777
        lsh     0,-042                 ; opcode bits 35..34
        jumpe   0,d6b_low_base_v1458+(d6b_literal_header_v1458-d6b_image_start_v1458)
        caie    0,1
        jrst    d6b_low_base_v1458+(d6b_match_header_v1458-d6b_image_start_v1458)
        jumpn   7,d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        setzm   0(12)
        cain    6,1                     ; skip AOJA for lengths > 1
        aoja    12,d6b_low_base_v1458+(d6b_command_v1458-d6b_image_start_v1458)    ; one-word zero run

d6b_zero_blt_v1458:
        hrlz    0,12                    ; source = zero seed
        hrri    0,1(12)                 ; destination after zero seed
        jrst    d6b_low_base_v1458+(d6b_copy_blt_v1458-d6b_image_start_v1458)

d6b_match_header_v1458:
        caie    0,2
        jrst    d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        movni   5,1(7)                 ; -(distance_field + 1)
        add     5,12                   ; match source address
        camge   5,14                   ; source must not precede origin
        jrst    d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        hrlz    0,5                    ; BLT source from match history

; The literal path enters here after updating its source/refill state.
d6b_copy_set_dest_v1458:
        hrr     0,12                   ; destination = output cursor
; The zero-run path enters here with a destination one past its seed.
d6b_copy_blt_v1458:
        add     12,6                   ; advance output cursor
        blt     0,-1(12)              ; copy through last destination
        jumpe   11,d6b_low_base_v1458+(d6b_command_v1458-d6b_image_start_v1458)

; A pending literal not completed by the current input window resumes
; from d6b_literal_v1458 after the VFS adapter supplies the next window.
d6b_need_input_v1458:
        movei   0,1
        popj    17,
d6b_error_v1458:
        seto    0,
        popj    17,
d6b_success_v1458:
        setz    0,
        popj    17,

d6b_literal_header_v1458:
        jumpn   7,d6b_low_base_v1458+(d6b_error_v1458-d6b_image_start_v1458)
        move    11,6                   ; literal words not yet copied
d6b_literal_v1458:
        jumpe   4,d6b_low_base_v1458+(d6b_need_input_v1458-d6b_image_start_v1458)
        move    6,11                   ; choose min(pending,available)
        camle   6,4
        move    6,4
        hrlz    0,3                    ; literal source from input window
        add     3,6                    ; update input pointer before BLT
        sub     4,6                    ; consume input window
        sub     11,6                   ; outstanding literal count
        jrst    d6b_low_base_v1458+(d6b_copy_set_dest_v1458-d6b_image_start_v1458)

        ; Byte pointer is part of the installed fixed-low-core image.
        ; LDB must not reference a Stage-1 literal-pool address.
        ; POINT 7,0,8 extracts bits 33..27 of AC0 (P=27, S=7).
d6b_length_pointer_v1458:
        .word   0330700000000

d6b_image_end_v1458:
