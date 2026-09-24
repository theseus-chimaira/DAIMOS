; Compact D6LZ36 decoder and resumable token core at fixed low core 000060.
;
; The complete image is position-fixed for d6lz_fixed_base.  Every PDP-6
; Stage1 copies d6lz_image_start..d6lz_image_end to that address.  KCORE links
; the same source first, so the running kernel uses exactly the same code.
;
; Public memory ABI (d6lz36_decode):
;   AC1 = destination
;   AC2 = destination words
;   AC3 = compressed source
;   AC4 = compressed source words
; Return AC1 = 0 on success, -1 on malformed/truncated input.
; AC4 is the number of unconsumed source words on successful return.
;
; Internal resumable ABI (d6lz36_decode_core):
;   AC1 = current destination
;   AC2 = destination words remaining
;   AC3 = current source window
;   AC4 = source-window words remaining
;   AC5 = destination base
;   AC6 = current control word
;   AC7 = control tokens remaining (zero means load a new control word)
; AC10 and AC11 are scratch and MUST already be saved by the caller.
; Return AC0 = 0 complete, +1 needs another source window, -1 malformed.
; All other state ACs are preserved across NEED_INPUT and may be resumed.
;
; Keeping the source-window mechanism in the common core lets the VFS decoder
; reuse all token/match logic while the memory/Stage1 path still enters only
; once for the complete compressed stream.

        .equ d6lz_fixed_base,000060
        .globl d6lz36_decode
        .globl d6lz36_decode_core

; Public memory-to-memory wrapper.  The core uses AC10/AC11 as match scratch,
; so preserve those two PDP-10 C ABI callee-save registers here.
d6lz36_decode:
d6lz_image_start:
        push    17,10
        push    17,11
        jumpe   2,d6lz_mem_success
        jumpe   1,d6lz_mem_error
        jumpe   3,d6lz_mem_error
        move    5,1
        setz    7,
        pushj   17,d6lz36_decode_core
        jumpe   0,d6lz_mem_success

d6lz_mem_error:
        seto    1,
        jrst    d6lz_mem_return

d6lz_mem_success:
        setz    1,
d6lz_mem_return:
        pop     17,11
        pop     17,10
        popj    17,

; Resumable decoder core.  A source-window exhaustion is not itself malformed:
; return +1 with the token/control state intact.  The memory wrapper converts
; that condition to its normal truncated-input error; VFS supplies a new
; window and re-enters here.
d6lz36_decode_core:
        jumpn   7,d6lz_have_control

d6lz_load_control:
        sojl    4,d6lz_core_need_input
        move    6,0(3)
        aoj     3,
        movei   7,044                  ; 36 control bits

d6lz_have_control:
        sojl    4,d6lz_core_need_input
        move    0,0(3)                 ; token
        aoj     3,
        jumpl   6,d6lz_match

d6lz_literal:
        movem   0,0(1)
        aoj     1,
        subi    2,1
        jrst    d6lz_token_done

d6lz_match:
        move    11,0                   ; descriptor bits 14..35 must be zero
        lsh     11,-016
        jumpn   11,d6lz_core_error

        move    10,0                   ; match source = dst - distance
        andi    10,0177
        aoj     10,
        movn    10,10
        add     10,1
        camge   10,5
        jrst    d6lz_core_error

        lsh     0,-7                   ; validated token leaves 7-bit length
        addi    0,3
        sub     2,0
        jumpl   2,d6lz_core_error

d6lz_match_loop:
        move    11,0(10)
        movem   11,0(1)
        aoj     10,
        aoj     1,
        sojg    0,d6lz_match_loop

d6lz_token_done:
        lsh     6,1
        jumpe   2,d6lz_core_success
        sojg    7,d6lz_have_control
        jrst    d6lz_load_control

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
