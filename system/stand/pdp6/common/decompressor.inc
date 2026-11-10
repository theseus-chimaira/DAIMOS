/**
 * @file decompressor.inc
 * @brief Fixed-address PDP-6 D6LZ36 resumable decoder shared by Stage-1/KCORE.
 *
 * Every Stage-1 carries this exact image and copies it to executive address
 * 000060 before invoking it. Internal transfers target that installed image,
 * so the source copy may reside anywhere. KCORE links the same source at
 * 000060 for runtime refill/decompression.
 *
 * Resumable ABI: AC12 destination cursor, AC13 output words remaining, AC3
 * source cursor, AC4 input words remaining, AC14 output base, AC10 control
 * word, AC11 control tokens remaining. AC6/AC7 are scratch. AC0 returns 0 on
 * completion, +1 for another source window, or -1 for malformed input.
 */
; Compact D6LZ36 resumable token core at fixed low core 000060.
;
; Every PDP-6 Stage1 carries these exact words and copies
; d6lz_image_start..d6lz_image_end to 000060.  Internal transfers are encoded
; against that fixed destination, so the installed copy is independently
; executable.  KCORE links the same source first at 000060 and the running
; VFS decoder resumes this exact core.
;
; Resumable ABI (d6lz36_decode_core):
;   AC12 = current destination
;   AC13 = destination words remaining
;   AC3 = current source window
;   AC4 = source-window words remaining
;   AC14 = destination base
;   AC10 = current control word
;   AC11 = control tokens remaining (zero means load a new control word)
; AC6 and AC7 are scratch.  A C-ABI caller preserves AC10/AC11 automatically.
; Return AC0 = 0 complete, +1 needs another source window, -1 malformed.
; All other state ACs are preserved across NEED_INPUT and may be resumed.
;
; Stage1 does not need the old generic memory wrapper: after installing this
; image it initializes the state ACs and calls 000060 directly.

        .equ d6lz_fixed_base,000060
        .globl d6lz36_decode_core

; Fixed low-core resumable decoder.  This source is also carried by every
; Stage1, but every internal transfer explicitly targets the installed image
; at 000060.  Consequently the copied Stage1 image is independently executable
; and KCORE links the identical words at their final address.
d6lz36_decode_core:
d6lz_image_start:
        jumpn   11,d6lz_fixed_base+(d6lz_have_control-d6lz_image_start)

d6lz_load_control:
        sojl    4,d6lz_fixed_base+(d6lz_core_need_input-d6lz_image_start)
        move    10,0(3)
        aoj     3,
        movei   11,044                  ; 36 control bits

d6lz_have_control:
        sojl    4,d6lz_fixed_base+(d6lz_core_need_input-d6lz_image_start)
        move    0,0(3)                 ; token
        aoj     3,
        jumpl   10,d6lz_fixed_base+(d6lz_match-d6lz_image_start)

d6lz_literal:
        movem   0,0(12)
        aoj     12,
        soja    13,d6lz_fixed_base+(d6lz_token_done-d6lz_image_start)

d6lz_match:
        move    7,0                    ; descriptor bits 14..35 must be zero
        lsh     7,-016
        jumpn   7,d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

        move    6,0                    ; match source = dst - distance
        andi    6,0177
        movni   6,1(6)
        add     6,12
        camge   6,14
        jrst    d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

        lsh     0,-7                   ; validated token leaves 7-bit length
        addi    0,3
        sub     13,0
        jumpl   13,d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

d6lz_match_loop:
        move    7,0(6)
        movem   7,0(12)
        aoj     6,
        aoj     12,
        sojg    0,d6lz_fixed_base+(d6lz_match_loop-d6lz_image_start)

d6lz_token_done:
        lsh     10,1
        jumpe   13,d6lz_fixed_base+(d6lz_core_success-d6lz_image_start)
        sojg    11,d6lz_fixed_base+(d6lz_have_control-d6lz_image_start)
        jrst    d6lz_fixed_base+(d6lz_load_control-d6lz_image_start)

d6lz_core_need_input:
        movei   0,1
        popj    17,
d6lz_core_error:
        seto    0,
        popj    17,
d6lz_core_success:
        setz    0,
        popj    17,

d6lz_image_end:
