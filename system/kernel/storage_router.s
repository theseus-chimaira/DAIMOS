; storage_router.s -- fixed Type-136 ownership boundary for split storage MRESes.
;
; DTC/MTC and DSK270 share the PDP-6 Type-136 data channel and therefore the
; PI3 data-completion and PI5 controller-status levels.  Only this small router
; is registered on those PI levels.  MINIT patches the two leaf jumps when the
; corresponding tape or disk MRES is installed.
;
; storage_state ownership codes are preserved from the combined driver:
;  -1 DTC read, -2 MTC read, -3 DSK read, -4 DSK write,
;  -5 MTC write, -6 DTC write.  Nonnegative state has no active controller.

        .text
        .globl storage_pi_handler
        .globl storage_dct_handler
        .globl storage_pi_dsk_jump
        .globl storage_pi_tape_jump
        .globl storage_dct_dsk_jump
        .globl storage_dct_tape_jump
        .globl storage_pi_return
        .globl storage_state
        .globl storage_iowd
        .globl storage_count
        .globl pdp10_pi_dispatch_done

storage_pi_handler:
        skipl storage_state
        jrst storage_pi_return
        move 2,storage_state
        aoje 2,storage_pi_tape_jump
        aoje 2,storage_pi_tape_jump
        aoje 2,storage_pi_dsk_jump
        aoje 2,storage_pi_dsk_jump
        jrst storage_pi_tape_jump

storage_dct_handler:
        move 2,storage_state
        aoje 2,storage_dct_tape_jump
        aoje 2,storage_dct_tape_jump
        aoje 2,storage_dct_dsk_jump
        aoje 2,storage_dct_dsk_jump
        jrst storage_dct_tape_jump

; MINIT rewrites only the RH target of these JRST words.
storage_pi_dsk_jump:
        jrst storage_pi_return
storage_pi_tape_jump:
        jrst storage_pi_return
storage_dct_dsk_jump:
        jrst storage_pi_return
storage_dct_tape_jump:
        jrst storage_pi_return

; PI3 and PI5 each have exactly one registered storage router.  Leaf drivers
; may therefore use AC2 internally and bypass the generic AOBJN fanout tail;
; the common PI return restores the interrupted ACs from low core.
storage_pi_return:
        jrst pdp10_pi_dispatch_done

        .bss
storage_state: .block 1
storage_iowd:  .block 1
storage_count: .block 1
