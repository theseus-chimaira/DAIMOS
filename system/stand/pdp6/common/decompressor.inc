; Compact D6LZ36 resumable token core at fixed low core 000060.
;
; Every PDP-6 Stage1 carries these exact words and copies
; d6lz_image_start..d6lz_image_end to 000060.  Internal transfers are encoded
; against that fixed destination, so the installed copy is independently
; executable.  KCORE links the same source first at 000060 and the running
; VFS decoder resumes this exact core.
;
; Resumable ABI (d6lz36_decode_core):
;   AC1 = current destination
;   AC2 = destination words remaining
;   AC3 = current source window
;   AC4 = source-window words remaining
;   AC5 = destination base
;   AC6 = current control word
;   AC7 = control tokens remaining (zero means load a new control word)
; AC10 and AC11 are scratch and MUST already be saved by a C-ABI caller.
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
        jumpn   7,d6lz_fixed_base+(d6lz_have_control-d6lz_image_start)

d6lz_load_control:
        sojl    4,d6lz_fixed_base+(d6lz_core_need_input-d6lz_image_start)
        move    6,0(3)
        aoj     3,
        movei   7,044                  ; 36 control bits

d6lz_have_control:
        sojl    4,d6lz_fixed_base+(d6lz_core_need_input-d6lz_image_start)
        move    0,0(3)                 ; token
        aoj     3,
        jumpl   6,d6lz_fixed_base+(d6lz_match-d6lz_image_start)

d6lz_literal:
        movem   0,0(1)
        aoj     1,
        subi    2,1
        jrst    d6lz_fixed_base+(d6lz_token_done-d6lz_image_start)

d6lz_match:
        move    11,0                   ; descriptor bits 14..35 must be zero
        lsh     11,-016
        jumpn   11,d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

        move    10,0                   ; match source = dst - distance
        andi    10,0177
        aoj     10,
        movn    10,10
        add     10,1
        camge   10,5
        jrst    d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

        lsh     0,-7                   ; validated token leaves 7-bit length
        addi    0,3
        sub     2,0
        jumpl   2,d6lz_fixed_base+(d6lz_core_error-d6lz_image_start)

d6lz_match_loop:
        move    11,0(10)
        movem   11,0(1)
        aoj     10,
        aoj     1,
        sojg    0,d6lz_fixed_base+(d6lz_match_loop-d6lz_image_start)

d6lz_token_done:
        lsh     6,1
        jumpe   2,d6lz_fixed_base+(d6lz_core_success-d6lz_image_start)
        sojg    7,d6lz_fixed_base+(d6lz_have_control-d6lz_image_start)
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
